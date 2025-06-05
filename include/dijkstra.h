#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "osm_reader.h"

// Estrutura para armazenar resultado do algoritmo de Dijkstra
typedef struct {
    long *caminho;          // Array com IDs dos pontos no caminho
    int tamanho_caminho;    // Número de pontos no caminho
    double distancia_total; // Distância total do caminho
    int sucesso;           // 1 se caminho foi encontrado, 0 caso contrário
} ResultadoDijkstra;

// Estrutura para nó na fila de prioridade
typedef struct {
    long ponto_id;
    double distancia;
} NoFilaPrioridade;

// Estrutura para fila de prioridade (min-heap)
typedef struct {
    NoFilaPrioridade *nos;
    int tamanho;
    int capacidade;
} FilaPrioridade;

// Funções para fila de prioridade
FilaPrioridade* criar_fila_prioridade(int capacidade);
void liberar_fila_prioridade(FilaPrioridade *fila);
void inserir_fila_prioridade(FilaPrioridade *fila, long ponto_id, double distancia);
NoFilaPrioridade extrair_minimo(FilaPrioridade *fila);
int fila_vazia(FilaPrioridade *fila);
void diminuir_chave(FilaPrioridade *fila, long ponto_id, double nova_distancia);

// Função principal do algoritmo de Dijkstra
ResultadoDijkstra* dijkstra(Grafo *grafo, long origem_id, long destino_id);

// Função para liberar resultado
void liberar_resultado_dijkstra(ResultadoDijkstra *resultado);

// Função auxiliar para encontrar índice de um ponto
int encontrar_indice_ponto(Grafo *grafo, long ponto_id);

// Função para reconstruir caminho
long* reconstruir_caminho(long *predecessores, int num_pontos, long origem_id, long destino_id, int *tamanho_caminho);

#endif // DIJKSTRA_H
