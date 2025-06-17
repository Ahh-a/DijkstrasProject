#include "ui_helpers.h"
#include <gtk/gtk.h>

// Função para atualizar o status bar
void update_status(AppData *app, const char *message) {
    if (!app || !app->statusbar || !message) return;
    
    gtk_statusbar_pop(GTK_STATUSBAR(app->statusbar), app->status_context_id);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context_id, message);
}

// Função para verificar se uma aresta faz parte do caminho mais curto
gboolean is_edge_in_shortest_path(AppData *app, long node1_id, long node2_id) {
    if (!app->has_shortest_path || !app->shortest_path || app->shortest_path_length < 2) {
        return FALSE;
    }
    
    // Verificar se os nós são consecutivos no caminho
    for (int i = 0; i < app->shortest_path_length - 1; i++) {
        long current_node = app->shortest_path[i];
        long next_node = app->shortest_path[i + 1];
        
        // Verificar em ambas as direções (grafo não direcionado)
        if ((current_node == node1_id && next_node == node2_id) ||
            (current_node == node2_id && next_node == node1_id)) {
            return TRUE;
        }
    }
    
    return FALSE;
}

// Função para atualizar as informações do arquivo
void update_file_info(AppData *app) {
    if (!app) return;
    
    if (app->file_label) {
        if (app->current_file) {
            gchar *basename = g_path_get_basename(app->current_file);
            gtk_label_set_text(GTK_LABEL(app->file_label), basename);
            g_free(basename);
        } else {
            gtk_label_set_text(GTK_LABEL(app->file_label), "No file loaded");
        }
    }
    
    if (app->stats_label) {
        if (app->grafo) {
            int oneway_count = count_oneway_edges(app->grafo);
            int bidirectional_count = count_bidirectional_edges(app->grafo);
            gchar *stats = g_strdup_printf("Points: %zu, Edges: %zu (Oneway: %d, Bidirectional: %d)", 
                                          app->grafo->num_pontos, 
                                          app->grafo->num_arestas,
                                          oneway_count,
                                          bidirectional_count);
            gtk_label_set_text(GTK_LABEL(app->stats_label), stats);
            g_free(stats);
        } else {
            gtk_label_set_text(GTK_LABEL(app->stats_label), "Points: 0, Edges: 0");
        }
    }
}

// Função para limpar dados da aplicação
void cleanup_app_data(AppData *app) {
    if (app->grafo) {
        liberar_grafo(app->grafo);
    }
    g_free(app->current_file);
}

// Função auxiliar para buscar um ponto pelo ID
Ponto* buscar_ponto_por_id(Grafo *grafo, long long id) {
    if (!grafo) return NULL;
    
    for (size_t i = 0; i < grafo->num_pontos; i++) {
        if (grafo->pontos[i].id == id) {
            return &grafo->pontos[i];
        }
    }
    return NULL;
}
