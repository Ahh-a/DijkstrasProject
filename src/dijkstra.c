#include "dijkstra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

// Implementação da fila de prioridade (min-heap)

FilaPrioridade* criar_fila_prioridade(int capacidade) {
    FilaPrioridade *fila = malloc(sizeof(FilaPrioridade));
    if (!fila) return NULL;
    
    fila->nos = malloc(sizeof(NoFilaPrioridade) * capacidade);
    if (!fila->nos) {
        free(fila);
        return NULL;
    }
    
    fila->tamanho = 0;
    fila->capacidade = capacidade;
    return fila;
}

void liberar_fila_prioridade(FilaPrioridade *fila) {
    if (fila) {
        free(fila->nos);
        free(fila);
    }
}

// Função auxiliar para trocar dois nós na fila
void trocar_nos(NoFilaPrioridade *a, NoFilaPrioridade *b) {
    NoFilaPrioridade temp = *a;
    *a = *b;
    *b = temp;
}

// Função para manter propriedade do min-heap (subindo)
void heapify_up(FilaPrioridade *fila, int indice) {
    if (indice == 0) return;
    
    int pai = (indice - 1) / 2;
    if (fila->nos[indice].distancia < fila->nos[pai].distancia) {
        trocar_nos(&fila->nos[indice], &fila->nos[pai]);
        heapify_up(fila, pai);
    }
}

// Função para manter propriedade do min-heap (descendo)
void heapify_down(FilaPrioridade *fila, int indice) {
    int menor = indice;
    int esquerda = 2 * indice + 1;
    int direita = 2 * indice + 2;
    
    if (esquerda < fila->tamanho && 
        fila->nos[esquerda].distancia < fila->nos[menor].distancia) {
        menor = esquerda;
    }
    
    if (direita < fila->tamanho && 
        fila->nos[direita].distancia < fila->nos[menor].distancia) {
        menor = direita;
    }
    
    if (menor != indice) {
        trocar_nos(&fila->nos[indice], &fila->nos[menor]);
        heapify_down(fila, menor);
    }
}

void inserir_fila_prioridade(FilaPrioridade *fila, long ponto_id, double distancia) {
    if (fila->tamanho >= fila->capacidade) return;
    
    fila->nos[fila->tamanho].ponto_id = ponto_id;
    fila->nos[fila->tamanho].distancia = distancia;
    heapify_up(fila, fila->tamanho);
    fila->tamanho++;
}

NoFilaPrioridade extrair_minimo(FilaPrioridade *fila) {
    NoFilaPrioridade minimo = fila->nos[0];
    fila->nos[0] = fila->nos[fila->tamanho - 1];
    fila->tamanho--;
    heapify_down(fila, 0);
    return minimo;
}

int fila_vazia(FilaPrioridade *fila) {
    return fila->tamanho == 0;
}

void diminuir_chave(FilaPrioridade *fila, long ponto_id, double nova_distancia) {
    // Encontrar o nó na fila
    for (int i = 0; i < fila->tamanho; i++) {
        if (fila->nos[i].ponto_id == ponto_id) {
            if (nova_distancia < fila->nos[i].distancia) {
                fila->nos[i].distancia = nova_distancia;
                heapify_up(fila, i);
            }
            break;
        }
    }
}

// Função auxiliar para encontrar índice de um ponto
int encontrar_indice_ponto(Grafo *grafo, long ponto_id) {
    for (size_t i = 0; i < grafo->num_pontos; i++) {
        if (grafo->pontos[i].id == ponto_id) {
            return i;
        }
    }
    return -1;
}

// Função para reconstruir caminho
long* reconstruir_caminho(long *predecessores, int num_pontos, long origem_id, long destino_id, int *tamanho_caminho) {
    // Contar quantos pontos há no caminho
    int contador = 0;
    long atual = destino_id;
    
    while (atual != -1) {
        contador++;
        int indice = -1;
        // Encontrar índice do ponto atual
        for (int i = 0; i < num_pontos; i++) {
            if (predecessores[i] != -1) {
                // Verificar se este é o ponto que estamos procurando
                // Precisamos de uma estrutura auxiliar para mapear IDs para índices
                break;
            }
        }
        if (atual == origem_id) break;
        atual = predecessores[indice]; // Isso precisa ser ajustado
    }
    
    // Alocar array para o caminho
    long *caminho = malloc(sizeof(long) * contador);
    if (!caminho) {
        *tamanho_caminho = 0;
        return NULL;
    }
    
    // Preencher caminho de trás para frente
    atual = destino_id;
    for (int i = contador - 1; i >= 0; i--) {
        caminho[i] = atual;
        if (atual == origem_id) break;
        // atual = predecessores[indice]; // Ajustar com mapeamento correto
    }
    
    *tamanho_caminho = contador;
    return caminho;
}

// Implementação principal do algoritmo de Dijkstra
ResultadoDijkstra* dijkstra(Grafo *grafo, long origem_id, long destino_id) {
    if (!grafo || grafo->num_pontos == 0) {
        return NULL;
    }
    
    // Encontrar índices dos pontos de origem e destino
    int indice_origem = encontrar_indice_ponto(grafo, origem_id);
    int indice_destino = encontrar_indice_ponto(grafo, destino_id);
    
    if (indice_origem == -1 || indice_destino == -1) {
        return NULL;
    }
    
    int num_pontos = grafo->num_pontos;
    
    // Inicializar arrays
    double *distancias = malloc(sizeof(double) * num_pontos);
    long *predecessores = malloc(sizeof(long) * num_pontos);
    int *visitados = malloc(sizeof(int) * num_pontos);
    
    if (!distancias || !predecessores || !visitados) {
        free(distancias);
        free(predecessores);
        free(visitados);
        return NULL;
    }
    
    // Inicializar valores
    for (int i = 0; i < num_pontos; i++) {
        distancias[i] = DBL_MAX;
        predecessores[i] = -1;
        visitados[i] = 0;
    }
    
    distancias[indice_origem] = 0.0;
    
    // Criar fila de prioridade
    FilaPrioridade *fila = criar_fila_prioridade(num_pontos);
    if (!fila) {
        free(distancias);
        free(predecessores);
        free(visitados);
        return NULL;
    }
    
    // Inserir ponto de origem na fila
    inserir_fila_prioridade(fila, origem_id, 0.0);
    
    // Executar algoritmo de Dijkstra
    while (!fila_vazia(fila)) {
        NoFilaPrioridade atual = extrair_minimo(fila);
        int indice_atual = encontrar_indice_ponto(grafo, atual.ponto_id);
        
        if (indice_atual == -1 || visitados[indice_atual]) {
            continue;
        }
        
        visitados[indice_atual] = 1;
        
        // Se chegamos ao destino, podemos parar
        if (atual.ponto_id == destino_id) {
            break;
        }
        
        // Examinar todos os vizinhos
        for (size_t i = 0; i < grafo->num_arestas; i++) {
            Aresta *aresta = &grafo->arestas[i];
            long vizinho_id = -1;
            
            // Determinar se esta aresta sai do ponto atual, respeitando direção
            if (aresta->origem == atual.ponto_id) {
                // Esta aresta sai do ponto atual
                vizinho_id = aresta->destino;
            } else if (aresta->destino == atual.ponto_id && aresta->is_bidirectional) {
                // Esta aresta chega ao ponto atual, mas só podemos usar na direção reversa se for bidirecional
                vizinho_id = aresta->origem;
            } else {
                continue; // Esta aresta não pode ser usada do ponto atual
            }
            
            int indice_vizinho = encontrar_indice_ponto(grafo, vizinho_id);
            if (indice_vizinho == -1 || visitados[indice_vizinho]) {
                continue;
            }
            
            double nova_distancia = distancias[indice_atual] + aresta->peso;
            
            if (nova_distancia < distancias[indice_vizinho]) {
                distancias[indice_vizinho] = nova_distancia;
                predecessores[indice_vizinho] = atual.ponto_id;
                inserir_fila_prioridade(fila, vizinho_id, nova_distancia);
            }
        }
    }
    
    // Criar resultado
    ResultadoDijkstra *resultado = malloc(sizeof(ResultadoDijkstra));
    if (!resultado) {
        liberar_fila_prioridade(fila);
        free(distancias);
        free(predecessores);
        free(visitados);
        return NULL;
    }
    
    // Verificar se caminho foi encontrado
    if (distancias[indice_destino] == DBL_MAX) {
        // Não há caminho
        resultado->caminho = NULL;
        resultado->tamanho_caminho = 0;
        resultado->distancia_total = -1.0;
        resultado->sucesso = 0;
    } else {
        // Reconstruir caminho completo usando predecessores
        // Primeiro, contar o tamanho do caminho
        int contador = 0;
        long atual_id = destino_id;
        int indice_atual = indice_destino;
        
        while (indice_atual != -1) {
            contador++;
            if (atual_id == origem_id) break;
            
            // Encontrar o predecessor
            long predecessor_id = predecessores[indice_atual];
            if (predecessor_id == -1) break;
            
            // Encontrar índice do predecessor
            indice_atual = encontrar_indice_ponto(grafo, predecessor_id);
            atual_id = predecessor_id;
        }
        
        // Alocar array para o caminho
        resultado->caminho = malloc(sizeof(long) * contador);
        if (resultado->caminho) {
            // Preencher caminho de trás para frente
            atual_id = destino_id;
            indice_atual = indice_destino;
            
            for (int i = contador - 1; i >= 0; i--) {
                resultado->caminho[i] = atual_id;
                if (atual_id == origem_id) break;
                
                // Próximo predecessor
                long predecessor_id = predecessores[indice_atual];
                if (predecessor_id == -1) break;
                
                indice_atual = encontrar_indice_ponto(grafo, predecessor_id);
                atual_id = predecessor_id;
            }
            
            resultado->tamanho_caminho = contador;
        } else {
            resultado->tamanho_caminho = 0;
        }
        resultado->distancia_total = distancias[indice_destino];
        resultado->sucesso = 1;
    }
    
    // Limpar memória
    liberar_fila_prioridade(fila);
    free(distancias);
    free(predecessores);
    free(visitados);
    
    return resultado;
}

void liberar_resultado_dijkstra(ResultadoDijkstra *resultado) {
    if (resultado) {
        free(resultado->caminho);
        free(resultado);
    }
}
