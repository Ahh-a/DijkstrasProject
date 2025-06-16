#ifndef EDIT_H
#define EDIT_H

#include "osm_reader.h"
#include <gtk/gtk.h>

// Enumeração para modos de edição
typedef enum {
    EDIT_MODE_NONE,      // Modo normal (navegação)
    EDIT_MODE_CREATE,    // Criar novos nós
    EDIT_MODE_DELETE,    // Deletar nós
    EDIT_MODE_CONNECT    // Conectar nós
} EditMode;

// Estrutura para armazenar estado da edição
typedef struct {
    EditMode current_mode;
    long long selected_node_id;      // Nó selecionado para operações
    long long connecting_from_id;    // Nó de origem para conexão
    gboolean has_selected_node;
    gboolean is_connecting;
    
    // Contador para IDs de novos nós
    long long next_node_id;
    long long next_way_id;
} EditState;

// Funções para gerenciamento do modo de edição
void init_edit_state(EditState *state);
void set_edit_mode(EditState *state, EditMode mode);
EditMode get_edit_mode(EditState *state);

// Funções para operações de edição
gboolean create_node_at_position(Grafo *grafo, EditState *state, double lat, double lon);
gboolean delete_node(Grafo *grafo, EditState *state, long long node_id);
gboolean connect_nodes(Grafo *grafo, EditState *state, long long from_id, long long to_id);
gboolean disconnect_nodes(Grafo *grafo, long long from_id, long long to_id);

// Funções auxiliares
long long find_highest_node_id(Grafo *grafo);
long long find_highest_way_id(Grafo *grafo);
void update_node_ids(EditState *state, Grafo *grafo);

// Funções para validação
gboolean can_delete_node(Grafo *grafo, long long node_id);
gboolean nodes_are_connected(Grafo *grafo, long long from_id, long long to_id);

// Funções para redimensionamento de arrays
gboolean resize_points_array(Grafo *grafo, size_t new_size);
gboolean resize_edges_array(Grafo *grafo, size_t new_size);

// Funções para salvar/carregar mudanças
gboolean save_graph_to_osm(Grafo *grafo, const char *filename);
void mark_graph_as_modified(Grafo *grafo);

// Funções para controle de direção das arestas
gboolean set_edge_bidirectional(Grafo *grafo, long long from_id, long long to_id, gboolean is_bidirectional);
gboolean is_edge_bidirectional(Grafo *grafo, long long from_id, long long to_id);
int count_oneway_edges(Grafo *grafo);
int count_bidirectional_edges(Grafo *grafo);

// Função para controlar direção de conexão entre dois nós
gboolean set_connection_direction(Grafo *grafo, long long node1, long long node2, int direction_type);

#endif // EDIT_H