#include "../include/osm_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

// Função principal para ler o arquivo OSM
typedef struct {
    long long id;
    double lat;
    double lon;
} NodeTemp;

typedef struct {
    long long origem;
    long long destino;
} EdgeTemp;

Grafo* ler_osm(const char* caminho_arquivo_osm) {
    FILE* f = fopen(caminho_arquivo_osm, "r");
    if (!f) return NULL;

    NodeTemp* nodes = NULL;
    size_t nodes_cap = 0, nodes_count = 0;
    EdgeTemp* edges = NULL;
    size_t edges_cap = 0, edges_count = 0;

    char linha[LINHA_MAX];
    int in_way = 0;
    long long way_nodes[128];
    int way_nodes_count = 0;

    while (fgets(linha, LINHA_MAX, f)) {
        // Parse node
        char* p = strstr(linha, "<node id=");
        if (p) {
            long long id;
            double lat, lon;
            if (sscanf(linha, " <node id=\"%lld\"%*[^l]lat=\"%lf\" lon=\"%lf\"", &id, &lat, &lon) == 3) {
                if (nodes_count == nodes_cap) {
                    nodes_cap = nodes_cap ? nodes_cap*2 : 1024;
                    nodes = realloc(nodes, nodes_cap * sizeof(NodeTemp));
                }
                nodes[nodes_count++] = (NodeTemp){id, lat, lon};
            }
            continue;
        }
        // Parse way
        if (strstr(linha, "<way ")) {
            in_way = 1;
            way_nodes_count = 0;
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
        if (in_way && strstr(linha, "</way>")) {
            // Adiciona arestas entre os nós do caminho
            for (int i = 1; i < way_nodes_count; ++i) {
                if (edges_count == edges_cap) {
                    edges_cap = edges_cap ? edges_cap*2 : 1024;
                    edges = realloc(edges, edges_cap * sizeof(EdgeTemp));
                }
                edges[edges_count++] = (EdgeTemp){way_nodes[i-1], way_nodes[i]};
                edges[edges_count++] = (EdgeTemp){way_nodes[i], way_nodes[i-1]}; // bidirecional
            }
            in_way = 0;
            continue;
        }
    }
    fclose(f);

    // Monta o grafo final
    Grafo* grafo = malloc(sizeof(Grafo));
    grafo->num_pontos = nodes_count;
    grafo->pontos = malloc(nodes_count * sizeof(Ponto));
    for (size_t i = 0; i < nodes_count; ++i) {
        grafo->pontos[i].id = nodes[i].id;
        grafo->pontos[i].lat = nodes[i].lat;
        grafo->pontos[i].lon = nodes[i].lon;
    }
    grafo->num_arestas = edges_count;
    grafo->arestas = malloc(edges_count * sizeof(Aresta));
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
        grafo->arestas[i].peso = haversine(lat1, lon1, lat2, lon2);
    }
    free(nodes);
    free(edges);
    return grafo;
}
