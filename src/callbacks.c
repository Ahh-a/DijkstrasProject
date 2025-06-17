#include "callbacks.h"
#include "ui_helpers.h"
#include "drawing.h"
#include "dijkstra.h"
#include <string.h>
#include <stdlib.h>

// Callback para abrir arquivo OSM
void on_open_osm_clicked(GtkMenuItem *menuitem, gpointer user_data) {
    (void)menuitem; // Suppress unused parameter warning
    AppData *app = (AppData *)user_data;
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open OSM File",
                                                   GTK_WINDOW(app->window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // Filtro para arquivos OSM
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "OpenStreetMap files");
    gtk_file_filter_add_pattern(filter, "*.osm");
    gtk_file_filter_add_pattern(filter, "*.xml");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        update_status(app, "Loading OSM file...");
        
        // Liberar grafo anterior se existir
        if (app->grafo) {
            liberar_grafo(app->grafo);
        }
        
        // Carregar novo arquivo
        app->grafo = ler_osm(filename);
        
        if (app->grafo) {
            g_free(app->current_file);
            app->current_file = g_strdup(filename);
            
            // Reset selected points when loading new file
            app->has_start_point = FALSE;
            app->has_end_point = FALSE;
            app->selected_start_id = 0;
            app->selected_end_id = 0;
            
            // Clear shortest path when loading new file
            if (app->shortest_path) {
                free(app->shortest_path);
                app->shortest_path = NULL;
            }
            app->has_shortest_path = FALSE;
            app->shortest_path_length = 0;
            
            // Reset zoom and pan
            app->zoom_factor = 1.0;
            app->pan_x = 0.0;
            app->pan_y = 0.0;
            
            // Update edit state with proper node IDs
            update_node_ids(&app->edit_state, app->grafo);
            
            update_file_info(app);
            update_status(app, "OSM file loaded successfully. Hold Ctrl and click points to select them.");
            
            // Redesenhar o grafo
            gtk_widget_queue_draw(app->graph_area);
        } else {
            update_status(app, "Error loading OSM file");
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                            GTK_DIALOG_MODAL,
                                                            GTK_MESSAGE_ERROR,
                                                            GTK_BUTTONS_OK,
                                                            "Error loading OSM file: %s",
                                                            filename);
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// Callback para load OSM via toolbar
void on_load_osm_clicked(GtkToolButton *toolbutton, gpointer user_data) {
    (void)toolbutton; // Suppress unused parameter warning
    on_open_osm_clicked(NULL, user_data);
}

// Callback para encontrar caminho mais curto
void on_find_path_clicked(GtkButton *button, gpointer user_data) {
    (void)button; // Suppress unused parameter warning
    AppData *app = (AppData *)user_data;
    
    if (!app->grafo) {
        update_status(app, "No graph loaded");
        return;
    }
    
    const char *start_text = gtk_entry_get_text(GTK_ENTRY(app->start_entry));
    const char *end_text = gtk_entry_get_text(GTK_ENTRY(app->end_entry));
    
    if (strlen(start_text) == 0 || strlen(end_text) == 0) {
        update_status(app, "Please enter start and end point IDs");
        return;
    }
    
    long start_id = strtol(start_text, NULL, 10);
    long end_id = strtol(end_text, NULL, 10);
    
    // Verificar se os pontos existem
    Ponto *start_point = buscar_ponto_por_id(app->grafo, start_id);
    Ponto *end_point = buscar_ponto_por_id(app->grafo, end_id);
    
    if (!start_point || !end_point) {
        update_status(app, "One or both points not found in graph");
        
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->results_text));
        gchar *error_msg = g_strdup_printf("Error: Point(s) not found\nStart ID: %ld %s\nEnd ID: %ld %s\n",
                                          start_id, start_point ? "(found)" : "(not found)",
                                          end_id, end_point ? "(found)" : "(not found)");
        gtk_text_buffer_set_text(buffer, error_msg, -1);
        g_free(error_msg);
        return;
    }
    
    update_status(app, "Calculating shortest path...");
    
    // Executar algoritmo de Dijkstra
    ResultadoDijkstra *resultado = dijkstra(app->grafo, start_id, end_id);
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->results_text));
    
    if (resultado && resultado->sucesso) {
        // Limpar caminho anterior se existir
        if (app->shortest_path) {
            free(app->shortest_path);
            app->shortest_path = NULL;
        }
        
        // Armazenar o novo caminho mais curto
        app->shortest_path_length = resultado->tamanho_caminho;
        app->shortest_path = malloc(app->shortest_path_length * sizeof(long));
        if (app->shortest_path) {
            for (int i = 0; i < resultado->tamanho_caminho; i++) {
                app->shortest_path[i] = resultado->caminho[i];
            }
            app->has_shortest_path = TRUE;
        } else {
            app->has_shortest_path = FALSE;
            app->shortest_path_length = 0;
        }
        
        gchar *result_text = g_strdup_printf("Shortest Path Found!\n\nStart Point:\n  ID: %lld\n  Lat: %.6f\n  Lon: %.6f\n\nEnd Point:\n  ID: %lld\n  Lat: %.6f\n  Lon: %.6f\n\nPath Details:\n  Distance: %.2f km\n  Points in path: %d\n\nPath: ",
                                            start_point->id, start_point->lat, start_point->lon,
                                            end_point->id, end_point->lat, end_point->lon,
                                            resultado->distancia_total, resultado->tamanho_caminho);
        
        // Adicionar pontos do caminho
        GString *path_str = g_string_new(result_text);
        for (int i = 0; i < resultado->tamanho_caminho; i++) {
            if (i > 0) g_string_append(path_str, " -> ");
            g_string_append_printf(path_str, "%ld", resultado->caminho[i]);
        }
        g_string_append(path_str, "\n");
        
        gtk_text_buffer_set_text(buffer, path_str->str, -1);
        
        g_free(result_text);
        g_string_free(path_str, TRUE);
        update_status(app, "Shortest path calculated successfully");
        
        // Redesenhar o grafo para mostrar o caminho em vermelho
        gtk_widget_queue_draw(app->graph_area);
        
        liberar_resultado_dijkstra(resultado);
    } else {
        // Limpar caminho anterior se não há caminho válido
        if (app->shortest_path) {
            free(app->shortest_path);
            app->shortest_path = NULL;
        }
        app->has_shortest_path = FALSE;
        app->shortest_path_length = 0;
        
        gchar *error_msg = g_strdup_printf("No path found between points!\n\nStart Point:\n  ID: %lld\n  Lat: %.6f\n  Lon: %.6f\n\nEnd Point:\n  ID: %lld\n  Lat: %.6f\n  Lon: %.6f\n\nThe points may be in disconnected components of the graph.\n",
                                          start_point->id, start_point->lat, start_point->lon,
                                          end_point->id, end_point->lat, end_point->lon);
        gtk_text_buffer_set_text(buffer, error_msg, -1);
        g_free(error_msg);
        update_status(app, "No path found");
        
        // Redesenhar o grafo
        gtk_widget_queue_draw(app->graph_area);
        
        if (resultado) {
            liberar_resultado_dijkstra(resultado);
        }
    }
}

// Callback para limpar
void on_clear_clicked(GtkToolButton *toolbutton, gpointer user_data) {
    (void)toolbutton; // Suppress unused parameter warning
    AppData *app = (AppData *)user_data;
    
    gtk_entry_set_text(GTK_ENTRY(app->start_entry), "");
    gtk_entry_set_text(GTK_ENTRY(app->end_entry), "");
    
    // Clear selected points
    app->has_start_point = FALSE;
    app->has_end_point = FALSE;
    app->selected_start_id = 0;
    app->selected_end_id = 0;
    
    // Clear shortest path
    if (app->shortest_path) {
        free(app->shortest_path);
        app->shortest_path = NULL;
    }
    app->has_shortest_path = FALSE;
    app->shortest_path_length = 0;
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->results_text));
    gtk_text_buffer_set_text(buffer, "", -1);
    
    update_status(app, "Cleared. Hold Ctrl and click points to select them.");
    
    // Redraw to remove highlighting
    if (app->grafo) {
        gtk_widget_queue_draw(app->graph_area);
    }
}

// Callback para sair
void on_quit_clicked(GtkMenuItem *menuitem, gpointer user_data) {
    (void)menuitem; // Suppress unused parameter warning
    (void)user_data; // Suppress unused parameter warning
    gtk_main_quit();
}

// Callback para fechamento da janela
gboolean on_window_delete(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    (void)widget; // Suppress unused parameter warning
    (void)event; // Suppress unused parameter warning
    AppData *app = (AppData *)user_data;
    cleanup_app_data(app);
    gtk_main_quit();
    return FALSE;
}

// Callback functions for edit mode buttons
void on_edit_create_clicked(GtkToolButton *toolbutton, gpointer user_data) {
    (void)toolbutton; // Suppress unused parameter warning
    AppData *app = (AppData *)user_data;
    
    app->edit_state.current_mode = EDIT_MODE_CREATE;
    app->edit_state.is_connecting = FALSE;
    app->edit_state.connecting_from_id = 0;
    
    update_status(app, "Create Mode: Click on empty space to create new nodes.");
    
    if (app->grafo) {
        gtk_widget_queue_draw(app->graph_area);
    }
}

void on_edit_delete_clicked(GtkToolButton *toolbutton, gpointer user_data) {
    (void)toolbutton; // Suppress unused parameter warning
    AppData *app = (AppData *)user_data;
    
    app->edit_state.current_mode = EDIT_MODE_DELETE;
    app->edit_state.is_connecting = FALSE;
    app->edit_state.connecting_from_id = 0;
    
    update_status(app, "Delete Mode: Click on nodes to delete them.");
    
    if (app->grafo) {
        gtk_widget_queue_draw(app->graph_area);
    }
}

void on_edit_connect_clicked(GtkToolButton *toolbutton, gpointer user_data) {
    (void)toolbutton; // Suppress unused parameter warning
    AppData *app = (AppData *)user_data;
    
    app->edit_state.current_mode = EDIT_MODE_CONNECT;
    app->edit_state.is_connecting = FALSE;
    app->edit_state.connecting_from_id = 0;
    
    update_status(app, "Connect Mode: Click two nodes to connect them.");
    
    if (app->grafo) {
        gtk_widget_queue_draw(app->graph_area);
    }
}

void on_edit_normal_clicked(GtkToolButton *toolbutton, gpointer user_data) {
    (void)toolbutton; // Suppress unused parameter warning
    AppData *app = (AppData *)user_data;
    
    app->edit_state.current_mode = EDIT_MODE_NONE;
    app->edit_state.is_connecting = FALSE;
    app->edit_state.connecting_from_id = 0;
    
    update_status(app, "Normal Mode: Hold Ctrl and click points to select them for Dijkstra.");
    
    if (app->grafo) {
        gtk_widget_queue_draw(app->graph_area);
    }
}

// Callback para exportar PNG
void on_export_png_clicked(GtkToolButton *toolbutton, gpointer user_data) {
    (void)toolbutton; // Suppress unused parameter warning
    AppData *app = (AppData *)user_data;
    
    if (!app->grafo) {
        update_status(app, "No graph loaded to export");
        GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                        GTK_DIALOG_MODAL,
                                                        GTK_MESSAGE_ERROR,
                                                        GTK_BUTTONS_OK,
                                                        "No graph data loaded. Please load an OSM file first.");
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        return;
    }
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Export Graph as PNG",
                                                   GTK_WINDOW(app->window),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Save", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // Configurar filtro para arquivos PNG
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "PNG images");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    // Nome padrão do arquivo
    if (app->current_file) {
        gchar *basename = g_path_get_basename(app->current_file);
        gchar *name_without_ext = g_strndup(basename, strlen(basename) - 4); // Remove .osm
        gchar *png_name = g_strdup_printf("%s_graph.png", name_without_ext);
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), png_name);
        g_free(basename);
        g_free(name_without_ext);
        g_free(png_name);
    } else {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "graph.png");
    }
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        // Adicionar extensão .png se não estiver presente
        char *final_filename;
        if (!g_str_has_suffix(filename, ".png")) {
            final_filename = g_strdup_printf("%s.png", filename);
        } else {
            final_filename = g_strdup(filename);
        }
        
        update_status(app, "Exporting graph to PNG...");
        
        // Exportar com resolução alta (1920x1080)
        render_graph_to_png(app, final_filename, 1920, 1080);
        
        gchar *success_msg = g_strdup_printf("Graph exported successfully to %s", final_filename);
        update_status(app, success_msg);
        
        // Mostrar diálogo de sucesso
        GtkWidget *success_dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                                          GTK_DIALOG_MODAL,
                                                          GTK_MESSAGE_INFO,
                                                          GTK_BUTTONS_OK,
                                                          "Graph exported successfully!\n\nSaved to: %s\nResolution: 1920x1080 pixels",
                                                          final_filename);
        gtk_dialog_run(GTK_DIALOG(success_dialog));
        gtk_widget_destroy(success_dialog);
        
        g_free(filename);
        g_free(final_filename);
        g_free(success_msg);
    }
    
    gtk_widget_destroy(dialog);
}
