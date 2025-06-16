#include "edit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Função para inicializar o estado de edição
void init_edit_state(EditState *state) {
    if (!state) return;
    
    state->current_mode = EDIT_MODE_NONE;
    state->selected_node_id = -1;
    state->connecting_from_id = -1;
    state->has_selected_node = FALSE;
    state->is_connecting = FALSE;
    state->next_node_id = 1;
    state->next_way_id = 1;
}

// Função para definir o modo de edição
void set_edit_mode(EditState *state, EditMode mode) {
    if (!state) return;
    
    // Limpar estado anterior
    state->selected_node_id = -1;
    state->connecting_from_id = -1;
    state->has_selected_node = FALSE;
    state->is_connecting = FALSE;
    
    state->current_mode = mode;
}

// Função para obter o modo atual
EditMode get_edit_mode(EditState *state) {
    return state ? state->current_mode : EDIT_MODE_NONE;
}

// Função para encontrar o maior ID de nó
long long find_highest_node_id(Grafo *grafo) {
    if (!grafo || grafo->num_pontos == 0) return 0;
    
    long long max_id = 0;
    for (size_t i = 0; i < grafo->num_pontos; i++) {
        if (grafo->pontos[i].id > max_id) {
            max_id = grafo->pontos[i].id;
        }
    }
    return max_id;
}

// Função para encontrar o maior ID de way
long long find_highest_way_id(Grafo *grafo) {
    if (!grafo || grafo->num_arestas == 0) return 0;
    
    long long max_id = 0;
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        // Assumindo que way_id está armazenado em algum lugar da aresta
        // Por simplicidade, vamos usar um contador incremental
        max_id = i + 100; // Começar IDs de ways em 100
    }
    return max_id;
}

// Função para atualizar IDs de nós
void update_node_ids(EditState *state, Grafo *grafo) {
    if (!state || !grafo) return;
    
    state->next_node_id = find_highest_node_id(grafo) + 1;
    state->next_way_id = find_highest_way_id(grafo) + 1;
}

// Função para redimensionar array de pontos
gboolean resize_points_array(Grafo *grafo, size_t new_size) {
    if (!grafo) return FALSE;
    
    Ponto *new_points = realloc(grafo->pontos, sizeof(Ponto) * new_size);
    if (!new_points) return FALSE;
    
    grafo->pontos = new_points;
    return TRUE;
}

// Função para redimensionar array de arestas
gboolean resize_edges_array(Grafo *grafo, size_t new_size) {
    if (!grafo) return FALSE;
    
    Aresta *new_edges = realloc(grafo->arestas, sizeof(Aresta) * new_size);
    if (!new_edges) return FALSE;
    
    grafo->arestas = new_edges;
    return TRUE;
}

// Função para criar um novo nó na posição especificada
gboolean create_node_at_position(Grafo *grafo, EditState *state, double lat, double lon) {
    if (!grafo || !state) return FALSE;
    
    // Redimensionar array se necessário
    if (!resize_points_array(grafo, grafo->num_pontos + 1)) {
        return FALSE;
    }
    
    // Criar novo ponto
    Ponto *new_point = &grafo->pontos[grafo->num_pontos];
    new_point->id = state->next_node_id;
    new_point->lat = lat;
    new_point->lon = lon;
    
    grafo->num_pontos++;
    state->next_node_id++;
    
    printf("Created new node with ID %lld at (%.6f, %.6f)\n", 
           new_point->id, lat, lon);
    
    return TRUE;
}

// Função para verificar se um nó pode ser deletado
gboolean can_delete_node(Grafo *grafo, long long node_id) {
    if (!grafo) return FALSE;
    
    // Contar quantas arestas conectam a este nó
    int connection_count = 0;
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        if (grafo->arestas[i].origem == node_id || 
            grafo->arestas[i].destino == node_id) {
            connection_count++;
        }
    }
    
    // Permitir deletar nós com até 2 conexões (nós de passagem)
    // Nós importantes (com muitas conexões) requerem confirmação
    return connection_count <= 2;
}

// Função para deletar um nó
gboolean delete_node(Grafo *grafo, EditState *state, long long node_id) {
    if (!grafo || !state) return FALSE;
    
    // Encontrar o índice do nó
    int node_index = -1;
    for (size_t i = 0; i < grafo->num_pontos; i++) {
        if (grafo->pontos[i].id == node_id) {
            node_index = i;
            break;
        }
    }
    
    if (node_index == -1) return FALSE;
    
    // Remover todas as arestas conectadas a este nó
    for (size_t i = 0; i < grafo->num_arestas; ) {
        if (grafo->arestas[i].origem == node_id || 
            grafo->arestas[i].destino == node_id) {
            // Mover última aresta para posição atual
            grafo->arestas[i] = grafo->arestas[grafo->num_arestas - 1];
            grafo->num_arestas--;
        } else {
            i++;
        }
    }
    
    // Remover o nó movendo o último para a posição atual
    grafo->pontos[node_index] = grafo->pontos[grafo->num_pontos - 1];
    grafo->num_pontos--;
    
    printf("Deleted node with ID %lld\n", node_id);
    return TRUE;
}

// Função para verificar se dois nós já estão conectados
gboolean nodes_are_connected(Grafo *grafo, long long from_id, long long to_id) {
    if (!grafo) return FALSE;
    
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        Aresta *edge = &grafo->arestas[i];
        // Verificar conexão direta
        if (edge->origem == from_id && edge->destino == to_id) {
            return TRUE;
        }
        // Verificar conexão reversa apenas se for bidirecional
        if (edge->origem == to_id && edge->destino == from_id && edge->is_bidirectional) {
            return TRUE;
        }
    }
    return FALSE;
}

// Função para calcular distância entre dois pontos (Haversine)
double calculate_distance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0; // Raio da Terra em km
    
    double dlat = (lat2 - lat1) * M_PI / 180.0;
    double dlon = (lon2 - lon1) * M_PI / 180.0;
    
    double a = sin(dlat/2) * sin(dlat/2) +
               cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
               sin(dlon/2) * sin(dlon/2);
    
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    return R * c;
}

// Função para conectar dois nós
gboolean connect_nodes(Grafo *grafo, EditState *state, long long from_id, long long to_id) {
    if (!grafo || !state || from_id == to_id) return FALSE;
    
    // Verificar se os nós existem
    Ponto *from_point = NULL, *to_point = NULL;
    for (size_t i = 0; i < grafo->num_pontos; i++) {
        if (grafo->pontos[i].id == from_id) from_point = &grafo->pontos[i];
        if (grafo->pontos[i].id == to_id) to_point = &grafo->pontos[i];
    }
    
    if (!from_point || !to_point) return FALSE;
    
    // Verificar se já estão conectados
    if (nodes_are_connected(grafo, from_id, to_id)) {
        printf("Nodes %lld and %lld are already connected\n", from_id, to_id);
        return FALSE;
    }
    
    // Redimensionar array de arestas se necessário
    if (!resize_edges_array(grafo, grafo->num_arestas + 1)) {
        return FALSE;
    }
    
    // Criar nova aresta
    Aresta *new_edge = &grafo->arestas[grafo->num_arestas];
    new_edge->origem = from_id;
    new_edge->destino = to_id;
    new_edge->is_bidirectional = 1;  // Por padrão, conexões criadas manualmente são bidirecionais
    
    // Calcular peso (distância)
    new_edge->peso = calculate_distance(from_point->lat, from_point->lon,
                                       to_point->lat, to_point->lon);
    
    grafo->num_arestas++;
    
    printf("Connected nodes %lld and %lld (distance: %.3f km)\n", 
           from_id, to_id, new_edge->peso);
    
    return TRUE;
}

// Função para desconectar dois nós
gboolean disconnect_nodes(Grafo *grafo, long long from_id, long long to_id) {
    if (!grafo) return FALSE;
    
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        Aresta *edge = &grafo->arestas[i];
        // Desconectar considerando direções
        if ((edge->origem == from_id && edge->destino == to_id) ||
            (edge->origem == to_id && edge->destino == from_id && edge->is_bidirectional)) {
            
            // Mover última aresta para posição atual
            grafo->arestas[i] = grafo->arestas[grafo->num_arestas - 1];
            grafo->num_arestas--;
            
            printf("Disconnected nodes %lld and %lld\n", from_id, to_id);
            return TRUE;
        }
    }
    
    return FALSE;
}

// Função para alterar a direção de uma conexão entre dois nós
gboolean set_connection_direction(Grafo *grafo, long long node1, long long node2, int direction_type) {
    // direction_type: 0 = oneway (node1 -> node2), 1 = bidirectional, -1 = oneway (node2 -> node1)
    if (!grafo) return FALSE;
    
    gboolean found_forward = FALSE, found_reverse = FALSE;
    size_t forward_index = 0, reverse_index = 0;
    
    // Encontrar arestas existentes
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        Aresta *edge = &grafo->arestas[i];
        if (edge->origem == node1 && edge->destino == node2) {
            found_forward = TRUE;
            forward_index = i;
        }
        if (edge->origem == node2 && edge->destino == node1) {
            found_reverse = TRUE;
            reverse_index = i;
        }
    }
    
    // Aplicar mudanças baseadas no tipo de direção
    if (direction_type == 1) { // Bidirectional
        if (found_forward) grafo->arestas[forward_index].is_bidirectional = 1;
        if (found_reverse) grafo->arestas[reverse_index].is_bidirectional = 1;
        printf("Connection %lld ↔ %lld set to bidirectional\n", node1, node2);
    } else if (direction_type == 0) { // Oneway node1 -> node2
        if (found_forward) grafo->arestas[forward_index].is_bidirectional = 0;
        if (found_reverse) grafo->arestas[reverse_index].is_bidirectional = 0;
        printf("Connection %lld → %lld set to oneway\n", node1, node2);
    } else if (direction_type == -1) { // Oneway node2 -> node1
        if (found_forward) grafo->arestas[forward_index].is_bidirectional = 0;
        if (found_reverse) grafo->arestas[reverse_index].is_bidirectional = 0;
        printf("Connection %lld → %lld set to oneway\n", node2, node1);
    }
    
    return found_forward || found_reverse;
}

// Função para definir se uma aresta é bidirecional ou não
gboolean set_edge_bidirectional(Grafo *grafo, long long from_id, long long to_id, gboolean is_bidirectional) {
    if (!grafo) return FALSE;
    
    gboolean found = FALSE;
    
    // Encontrar e modificar a aresta principal
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        Aresta *edge = &grafo->arestas[i];
        if (edge->origem == from_id && edge->destino == to_id) {
            edge->is_bidirectional = is_bidirectional ? 1 : 0;
            found = TRUE;
            break;
        }
    }
    
    // Se encontrou a aresta principal e está tornando bidirecional,
    // verificar se existe aresta reversa e também modificá-la
    if (found && is_bidirectional) {
        for (size_t i = 0; i < grafo->num_arestas; i++) {
            Aresta *edge = &grafo->arestas[i];
            if (edge->origem == to_id && edge->destino == from_id) {
                edge->is_bidirectional = 1;
                break;
            }
        }
    }
    
    if (found) {
        printf("Edge %lld -> %lld set to %s\n", from_id, to_id, 
               is_bidirectional ? "bidirectional" : "oneway");
    }
    
    return found;
}

// Função para verificar se uma aresta é bidirecional
gboolean is_edge_bidirectional(Grafo *grafo, long long from_id, long long to_id) {
    if (!grafo) return FALSE;
    
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        Aresta *edge = &grafo->arestas[i];
        if (edge->origem == from_id && edge->destino == to_id) {
            return edge->is_bidirectional;
        }
    }
    
    return FALSE;
}

// Função para contar arestas de mão única
int count_oneway_edges(Grafo *grafo) {
    if (!grafo) return 0;
    
    int count = 0;
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        if (!grafo->arestas[i].is_bidirectional) {
            count++;
        }
    }
    return count;
}

// Função para contar arestas bidirecionais
int count_bidirectional_edges(Grafo *grafo) {
    if (!grafo) return 0;
    
    int count = 0;
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        if (grafo->arestas[i].is_bidirectional) {
            count++;
        }
    }
    return count;
}

// Função para marcar grafo como modificado
void mark_graph_as_modified(Grafo *grafo) {
    // Esta função pode ser expandida para adicionar metadados de modificação
    if (grafo) {
        printf("Graph marked as modified\n");
    }
}

// Função para salvar grafo em formato OSM (implementação básica)
gboolean save_graph_to_osm(Grafo *grafo, const char *filename) {
    if (!grafo || !filename) return FALSE;
    
    FILE *file = fopen(filename, "w");
    if (!file) return FALSE;
    
    // Escrever cabeçalho OSM
    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(file, "<osm version=\"0.6\" generator=\"DijkstraEditor\">\n");
    
    // Escrever nós
    for (size_t i = 0; i < grafo->num_pontos; i++) {
        Ponto *p = &grafo->pontos[i];
        fprintf(file, "  <node id=\"%lld\" lat=\"%.7f\" lon=\"%.7f\" />\n",
                p->id, p->lat, p->lon);
    }
    
    // Escrever ways (simplificado - cada aresta vira um way)
    long way_id = 1000;
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        Aresta *a = &grafo->arestas[i];
        fprintf(file, "  <way id=\"%ld\">\n", way_id++);
        fprintf(file, "    <nd ref=\"%lld\"/>\n", a->origem);
        fprintf(file, "    <nd ref=\"%lld\"/>\n", a->destino);
        fprintf(file, "    <tag k=\"highway\" v=\"unclassified\"/>\n");
        // Adicionar tag oneway se for direcional
        if (!a->is_bidirectional) {
            fprintf(file, "    <tag k=\"oneway\" v=\"yes\"/>\n");
        }
        fprintf(file, "  </way>\n");
    }
    
    fprintf(file, "</osm>\n");
    fclose(file);
    
    printf("Graph saved to %s\n", filename);
    return TRUE;
}