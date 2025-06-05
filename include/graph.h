#ifndef GRAPH_H
#define GRAPH_H

#include "osm_reader.h"
#include <stddef.h>
#include <sys/types.h> // Para ssize_t

// Cria um grafo vazio
Grafo* criar_grafo(void);

// Adiciona um ponto ao grafo
int adicionar_ponto(Grafo* grafo, long long id, double lat, double lon);

// Adiciona uma aresta ao grafo
int adicionar_aresta(Grafo* grafo, long long origem, long long destino, double peso);

// Busca o índice de um ponto pelo id
ssize_t buscar_indice_ponto(const Grafo* grafo, long long id);

// Imprime o grafo (para debug)
void imprimir_grafo(const Grafo* grafo);

#endif // GRAPH_H
