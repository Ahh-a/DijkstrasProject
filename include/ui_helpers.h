#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include "app_data.h"

// Função para atualizar o status bar
void update_status(AppData *app, const char *message);

// Função para verificar se uma aresta faz parte do caminho mais curto
gboolean is_edge_in_shortest_path(AppData *app, long node1_id, long node2_id);

// Função para atualizar as informações do arquivo
void update_file_info(AppData *app);

// Função para limpar dados da aplicação
void cleanup_app_data(AppData *app);

// Função auxiliar para buscar um ponto pelo ID
Ponto* buscar_ponto_por_id(Grafo *grafo, long long id);

#endif // UI_HELPERS_H
