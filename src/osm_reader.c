#include "../include/osm_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>

#define _GNU_SOURCE

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define LINHA_MAX 1024

// Função auxiliar para calcular a distância entre dois pontos (Haversine)
static double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dlat = (lat2 - lat1) * M_PI / 180.0;
    double dlon = (lon2 - lon1) * M_PI / 180.0;
    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;
    double a = pow(sin(dlat/2),2) + cos(lat1)*cos(lat2)*pow(sin(dlon/2),2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double R = 6371000.0; // Raio da Terra em metros
    return R * c;
}

// Função para liberar o grafo
void liberar_grafo(Grafo* grafo) {
    if (grafo) {
        free(grafo->pontos);
        free(grafo->arestas);
        free(grafo);
    }
}


// Struct temporária para armazenar nós durante a leitura do OSM
typedef struct {
    long long id;
    double lat;
    double lon;
} NodeTemp;

// Struct temporária para armazenar arestas durante a leitura do OSM
typedef struct {
    long long origem;
    long long destino;
    int is_bidirectional; // 1 se bidirecional, 0 se mão única
} EdgeTemp;

// Função principal para ler o arquivo OSM
Grafo* ler_osm(const char* caminho_arquivo_osm) {
    // Set C locale to ensure decimal point parsing
    setlocale(LC_NUMERIC, "C");
    
    FILE* f = fopen(caminho_arquivo_osm, "r");
    if (!f) {
        return NULL;
    }

    NodeTemp* nodes = NULL;
    size_t nodes_cap = 0, nodes_count = 0;
    EdgeTemp* edges = NULL;
    size_t edges_cap = 0, edges_count = 0;

    char linha[LINHA_MAX];
    int in_way = 0;
    long long way_nodes[128];
    int way_nodes_count = 0;
    int is_oneway = 0;  // Flag para detectar vias de mão única

    while (fgets(linha, LINHA_MAX, f)) {
        // Parse node - improved parsing for complex OSM format
        char* p = strstr(linha, "<node id=");
        if (p) {
            long long id;
            double lat, lon;
            int parsed = 0;
            
            // Extract manually using string search for robust parsing
            char *id_start = strstr(linha, "id=\"");
            char *lat_start = strstr(linha, "lat=\"");
            char *lon_start = strstr(linha, "lon=\"");
            
            if (id_start && lat_start && lon_start) {
                id_start += 4; // skip 'id="'
                lat_start += 5; // skip 'lat="'
                lon_start += 5; // skip 'lon="'
                
                char id_str[32], lat_str[32], lon_str[32];
                
                // Extract ID
                int i = 0;
                while (id_start[i] != '"' && i < 31 && id_start[i] != '\0') {
                    id_str[i] = id_start[i];
                    i++;
                }
                id_str[i] = '\0';
                
                // Extract latitude
                i = 0;
                while (lat_start[i] != '"' && i < 31 && lat_start[i] != '\0') {
                    lat_str[i] = lat_start[i];
                    i++;
                }
                lat_str[i] = '\0';
                
                // Extract longitude
                i = 0;
                while (lon_start[i] != '"' && i < 31 && lon_start[i] != '\0') {
                    lon_str[i] = lon_start[i];
                    i++;
                }
                lon_str[i] = '\0';
                
                id = strtoll(id_str, NULL, 10);
                lat = strtod(lat_str, NULL);
                lon = strtod(lon_str, NULL);
                parsed = 1;
            }
            
            if (parsed) {
                if (nodes_count == nodes_cap) {
                    nodes_cap = nodes_cap ? nodes_cap*2 : 1024;
                    NodeTemp* new_nodes = realloc(nodes, nodes_cap * sizeof(NodeTemp));
                    if (!new_nodes) {
                        free(nodes);
                        free(edges);
                        fclose(f);
                        return NULL;
                    }
                    nodes = new_nodes;
                }
                nodes[nodes_count++] = (NodeTemp){id, lat, lon};
            }
            continue;
        }
        
        // Parse way
        if (strstr(linha, "<way ")) {
            in_way = 1;
            way_nodes_count = 0;
            is_oneway = 0;  // Reset flag para cada way
            continue;
        }
        if (in_way && strstr(linha, "<nd ref=")) {
            long long ref;
            if (sscanf(linha, " <nd ref=\"%lld\"", &ref) == 1) {
                if (way_nodes_count < 128)
                    way_nodes[way_nodes_count++] = ref;
            }
            continue;
        }
        // Detectar tags de via única
        if (in_way && strstr(linha, "<tag")) {
            if (strstr(linha, "k=\"oneway\"")) {
                if (strstr(linha, "v=\"yes\"") || strstr(linha, "v=\"true\"") || strstr(linha, "v=\"1\"")) {
                    is_oneway = 1;
                } else if (strstr(linha, "v=\"-1\"") || strstr(linha, "v=\"reverse\"")) {
                    is_oneway = -1;  // Via única na direção reversa
                }
            }
            continue;
        }
        if (in_way && strstr(linha, "</way>")) {
            // Adiciona arestas entre os nós do caminho
            for (int i = 1; i < way_nodes_count; ++i) {
                // Check if we need more space (considering we might add 2 edges)
                if (edges_count + 1 >= edges_cap) {
                    edges_cap = edges_cap ? edges_cap*2 : 1024;
                    EdgeTemp* new_edges = realloc(edges, edges_cap * sizeof(EdgeTemp));
                    if (!new_edges) {
                        free(nodes);
                        free(edges);
                        fclose(f);
                        return NULL;
                    }
                    edges = new_edges;
                }
                
                if (is_oneway == 0) {
                    // Via bidirecional - adicionar ambas as direções
                    if (edges_count + 1 >= edges_cap) {
                        edges_cap = edges_cap*2;
                        EdgeTemp* new_edges = realloc(edges, edges_cap * sizeof(EdgeTemp));
                        if (!new_edges) {
                            free(nodes);
                            free(edges);
                            fclose(f);
                            return NULL;
                        }
                        edges = new_edges;
                    }
                    edges[edges_count++] = (EdgeTemp){way_nodes[i-1], way_nodes[i], 1};
                    edges[edges_count++] = (EdgeTemp){way_nodes[i], way_nodes[i-1], 1};
                } else if (is_oneway == 1) {
                    // Via de mão única normal
                    edges[edges_count++] = (EdgeTemp){way_nodes[i-1], way_nodes[i], 0};
                } else if (is_oneway == -1) {
                    // Via de mão única reversa
                    edges[edges_count++] = (EdgeTemp){way_nodes[i], way_nodes[i-1], 0};
                }
            }
            in_way = 0;
            continue;
        }
    }
    fclose(f);

    // Monta o grafo final
    Grafo* grafo = malloc(sizeof(Grafo));
    if (!grafo) {
        free(nodes);
        free(edges);
        return NULL;
    }
    
    grafo->num_pontos = nodes_count;
    grafo->pontos = malloc(nodes_count * sizeof(Ponto));
    if (!grafo->pontos) {
        free(grafo);
        free(nodes);
        free(edges);
        return NULL;
    }
    
    for (size_t i = 0; i < nodes_count; ++i) {
        grafo->pontos[i].id = nodes[i].id;
        grafo->pontos[i].lat = nodes[i].lat;
        grafo->pontos[i].lon = nodes[i].lon;
    }
    
    grafo->num_arestas = edges_count;
    grafo->arestas = malloc(edges_count * sizeof(Aresta));
    if (!grafo->arestas) {
        free(grafo->pontos);
        free(grafo);
        free(nodes);
        free(edges);
        return NULL;
    }
    for (size_t i = 0; i < edges_count; ++i) {
        // Busca os pontos de origem e destino
        double lat1 = 0, lon1 = 0, lat2 = 0, lon2 = 0;
        for (size_t j = 0; j < nodes_count; ++j) {
            if (nodes[j].id == edges[i].origem) {
                lat1 = nodes[j].lat;
                lon1 = nodes[j].lon;
            }
            if (nodes[j].id == edges[i].destino) {
                lat2 = nodes[j].lat;
                lon2 = nodes[j].lon;
            }
        }
        grafo->arestas[i].origem = edges[i].origem;
        grafo->arestas[i].destino = edges[i].destino;
        grafo->arestas[i].peso = haversine(lat1, lon1, lat2, lon2) / 1000.0; // Convert to km
        grafo->arestas[i].is_bidirectional = edges[i].is_bidirectional;
    }
    
    free(nodes);
    free(edges);
    
    return grafo;
}
