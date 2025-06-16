#include "../include/graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Cria um grafo vazio
Grafo* criar_grafo(void) {
    Grafo* grafo = malloc(sizeof(Grafo));
    if (!grafo) return NULL;
    grafo->pontos = NULL;
    grafo->num_pontos = 0;
    grafo->arestas = NULL;
    grafo->num_arestas = 0;
    return grafo;
}

// Adiciona um ponto ao grafo
int adicionar_ponto(Grafo* grafo, long long id, double lat, double lon) {
    if (!grafo) return -1;
    Ponto* novo = realloc(grafo->pontos, (grafo->num_pontos + 1) * sizeof(Ponto));
    if (!novo) return -1;
    grafo->pontos = novo;
    grafo->pontos[grafo->num_pontos].id = id;
    grafo->pontos[grafo->num_pontos].lat = lat;
    grafo->pontos[grafo->num_pontos].lon = lon;
    grafo->num_pontos++;
    return 0;
}

// Adiciona uma aresta ao grafo
int adicionar_aresta(Grafo* grafo, long long origem, long long destino, double peso) {
    if (!grafo) return -1;
    Aresta* novo = realloc(grafo->arestas, (grafo->num_arestas + 1) * sizeof(Aresta));
    if (!novo) return -1;
    grafo->arestas = novo;
    grafo->arestas[grafo->num_arestas].origem = origem;
    grafo->arestas[grafo->num_arestas].destino = destino;
    grafo->arestas[grafo->num_arestas].peso = peso;
    grafo->arestas[grafo->num_arestas].is_bidirectional = 1; // Por padrão, bidirectional
    grafo->num_arestas++;
    return 0;
}

// Busca o índice de um ponto pelo id
ssize_t buscar_indice_ponto(const Grafo* grafo, long long id) {
    if (!grafo) return -1;
    for (size_t i = 0; i < grafo->num_pontos; ++i) {
        if (grafo->pontos[i].id == id)
            return (ssize_t)i;
    }
    return -1;
}

// Imprime o grafo (para debug)
void imprimir_grafo(const Grafo* grafo) {
    if (!grafo) return;
    printf("Pontos (%zu):\n", grafo->num_pontos);
    for (size_t i = 0; i < grafo->num_pontos; ++i) {
        printf("  [%zu] id=%lld lat=%lf lon=%lf\n", i, grafo->pontos[i].id, grafo->pontos[i].lat, grafo->pontos[i].lon);
    }
    printf("Arestas (%zu):\n", grafo->num_arestas);
    for (size_t i = 0; i < grafo->num_arestas; ++i) {
        printf("  [%zu] origem=%lld destino=%lld peso=%.2lf %s\n", 
               i, grafo->arestas[i].origem, grafo->arestas[i].destino, 
               grafo->arestas[i].peso,
               grafo->arestas[i].is_bidirectional ? "(bidirectional)" : "(oneway)");
    }
}
