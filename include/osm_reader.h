#ifndef OSM_READER_H
#define OSM_READER_H

#include <stddef.h>

// Estrutura para armazenar um ponto (nó)
typedef struct {
    long long id;
    double lat;
    double lon;
} Ponto;

// Estrutura para armazenar uma aresta
typedef struct {
    long long origem;
    long long destino;
    double peso;
    int is_bidirectional;  // 1 para bidirectional (mão dupla), 0 para direcional (mão única)
} Aresta;

// Estrutura para armazenar o grafo
typedef struct {
    Ponto* pontos;
    size_t num_pontos;
    Aresta* arestas;
    size_t num_arestas;
} Grafo;

// Protótipo da função que lê o arquivo OSM e armazena os pontos e as arestas
// O grafo deve ser alocado dinamicamente dentro da função e retornado por ponteiro
Grafo* ler_osm(const char* caminho_arquivo_osm);

// Função para liberar a memória do grafo
void liberar_grafo(Grafo* grafo);

#endif // OSM_READER_H
