#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "osm_reader.h"
#include "graph.h"
#include "dijkstra.h"

// Forward declarations
Ponto* buscar_ponto_por_id(Grafo *grafo, long long id);

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Estrutura para armazenar dados da aplicação
typedef struct {
    GtkWidget *window;
    GtkWidget *file_label;
    GtkWidget *stats_label;
    GtkWidget *start_entry;
    GtkWidget *end_entry;
    GtkWidget *results_text;
    GtkWidget *graph_area;
    GtkWidget *statusbar;
    guint status_context_id;
    
    Grafo *grafo;
    char *current_file;
    
    // Variáveis para zoom e pan
    double zoom_factor;
    double pan_x, pan_y;
    double last_mouse_x, last_mouse_y;
    gboolean dragging;
    
    // Variáveis para seleção de pontos
    long long selected_start_id;
    long long selected_end_id;
    gboolean has_start_point;
    gboolean has_end_point;
} AppData;

// Função para atualizar o status bar
void update_status(AppData *app, const char *message) {
    gtk_statusbar_pop(GTK_STATUSBAR(app->statusbar), app->status_context_id);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context_id, message);
}

// Função para atualizar as informações do arquivo
void update_file_info(AppData *app) {
    if (app->current_file) {
        gchar *basename = g_path_get_basename(app->current_file);
        gtk_label_set_text(GTK_LABEL(app->file_label), basename);
        g_free(basename);
    } else {
        gtk_label_set_text(GTK_LABEL(app->file_label), "No file loaded");
    }
    
    if (app->grafo) {
        gchar *stats = g_strdup_printf("Points: %zu, Edges: %zu", 
                                      app->grafo->num_pontos, 
                                      app->grafo->num_arestas);
        gtk_label_set_text(GTK_LABEL(app->stats_label), stats);
        g_free(stats);
    } else {
        gtk_label_set_text(GTK_LABEL(app->stats_label), "Points: 0, Edges: 0");
    }
}

// Função para desenhar o grafo
gboolean on_graph_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Clear with white background
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    // If no graph data, show message
    if (!app->grafo) {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 20, allocation.height / 2);
        cairo_show_text(cr, "No graph data loaded. Use File -> Open OSM to load a file.");
        cairo_move_to(cr, 20, allocation.height / 2 + 20);
        cairo_show_text(cr, "After loading, hold Ctrl and click points to select them for Dijkstra.");
        return TRUE;
    }
    
    // Find bounds
    double min_lat = 90.0, max_lat = -90.0;
    double min_lon = 180.0, max_lon = -180.0;
    
    for (size_t i = 0; i < app->grafo->num_pontos; i++) {
        Ponto *p = &app->grafo->pontos[i];
        if (p->lat < min_lat) min_lat = p->lat;
        if (p->lat > max_lat) max_lat = p->lat;
        if (p->lon < min_lon) min_lon = p->lon;
        if (p->lon > max_lon) max_lon = p->lon;
    }
    
    double lat_range = max_lat - min_lat;
    double lon_range = max_lon - min_lon;
    
    if (lat_range > 0.0 && lon_range > 0.0) {
        // Calculate base scale to fit in the area
        double scale_x = allocation.width / lon_range;
        double scale_y = allocation.height / lat_range;
        double base_scale = (scale_x < scale_y) ? scale_x : scale_y;
        base_scale *= 0.9; // Leave some margin
        
        // Apply zoom factor
        double scale = base_scale * app->zoom_factor;
        
        // Apply pan offset
        double center_x = allocation.width / 2.0;
        double center_y = allocation.height / 2.0;
        double map_center_x = (min_lon + max_lon) / 2.0;
        double map_center_y = (min_lat + max_lat) / 2.0;
        
        // Draw edges first (behind points)
        cairo_set_source_rgba(cr, 0.4, 0.4, 0.4, 0.6);
        cairo_set_line_width(cr, 0.8 / app->zoom_factor); // Thinner lines when zoomed in
        
        for (size_t i = 0; i < app->grafo->num_arestas; i++) {
            Aresta *a = &app->grafo->arestas[i];
            
            // Find source and destination points
            Ponto *origem = NULL, *destino = NULL;
            
            for (size_t j = 0; j < app->grafo->num_pontos; j++) {
                if (app->grafo->pontos[j].id == a->origem) {
                    origem = &app->grafo->pontos[j];
                }
                if (app->grafo->pontos[j].id == a->destino) {
                    destino = &app->grafo->pontos[j];
                }
                if (origem && destino) break;
            }
            
            if (origem && destino) {
                double x1 = center_x + (origem->lon - map_center_x) * scale + app->pan_x;
                double y1 = center_y - (origem->lat - map_center_y) * scale + app->pan_y;
                double x2 = center_x + (destino->lon - map_center_x) * scale + app->pan_x;
                double y2 = center_y - (destino->lat - map_center_y) * scale + app->pan_y;
                
                // Only draw if at least part of the line is visible
                if ((x1 >= -10 || x2 >= -10) && (x1 <= allocation.width + 10 || x2 <= allocation.width + 10) &&
                    (y1 >= -10 || y2 >= -10) && (y1 <= allocation.height + 10 || y2 <= allocation.height + 10)) {
                    cairo_move_to(cr, x1, y1);
                    cairo_line_to(cr, x2, y2);
                    cairo_stroke(cr);
                }
            }
        }
        
        // Draw graph points (on top of edges)
        for (size_t i = 0; i < app->grafo->num_pontos; i++) {
            Ponto *p = &app->grafo->pontos[i];
            double x = center_x + (p->lon - map_center_x) * scale + app->pan_x;
            double y = center_y - (p->lat - map_center_y) * scale + app->pan_y;
            
            // Only draw if point is visible
            if (x >= -10 && x <= allocation.width + 10 && y >= -10 && y <= allocation.height + 10) {
                double point_radius = 2.0 / app->zoom_factor;
                if (point_radius < 1.0) point_radius = 1.0;
                if (point_radius > 3.0) point_radius = 3.0;
                
                // Check if this point is selected
                gboolean is_start = (app->has_start_point && p->id == app->selected_start_id);
                gboolean is_end = (app->has_end_point && p->id == app->selected_end_id);
                
                if (is_start) {
                    // Start point - green
                    cairo_set_source_rgb(cr, 0.2, 0.8, 0.2);
                    cairo_arc(cr, x, y, point_radius + 2, 0, 2 * M_PI);
                    cairo_fill(cr);
                } else if (is_end) {
                    // End point - red
                    cairo_set_source_rgb(cr, 0.8, 0.2, 0.2);
                    cairo_arc(cr, x, y, point_radius + 2, 0, 2 * M_PI);
                    cairo_fill(cr);
                } else {
                    // Normal point - purple
                    cairo_set_source_rgb(cr, 0.8, 0.2, 0.8);
                    cairo_arc(cr, x, y, point_radius, 0, 2 * M_PI);
                    cairo_fill(cr);
                }
            }
        }
    }
    
    return TRUE;
}

// Callback para zoom com scroll do mouse
gboolean on_graph_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    
    if (!app->grafo) return FALSE;
    
    double zoom_factor = 1.1;
    
    if (event->direction == GDK_SCROLL_UP) {
        app->zoom_factor *= zoom_factor;
    } else if (event->direction == GDK_SCROLL_DOWN) {
        app->zoom_factor /= zoom_factor;
    }
    
    // Limitar o zoom
    if (app->zoom_factor < 0.1) app->zoom_factor = 0.1;
    if (app->zoom_factor > 50.0) app->zoom_factor = 50.0;
    
    gtk_widget_queue_draw(widget);
    return TRUE;
}

// Função para encontrar o ponto mais próximo do clique
Ponto* find_closest_point(AppData *app, double click_x, double click_y) {
    if (!app->grafo) return NULL;
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(app->graph_area, &allocation);
    
    // Calculate bounds and scale (same as drawing function)
    double min_lat = 90.0, max_lat = -90.0;
    double min_lon = 180.0, max_lon = -180.0;
    
    for (size_t i = 0; i < app->grafo->num_pontos; i++) {
        Ponto *p = &app->grafo->pontos[i];
        if (p->lat < min_lat) min_lat = p->lat;
        if (p->lat > max_lat) max_lat = p->lat;
        if (p->lon < min_lon) min_lon = p->lon;
        if (p->lon > max_lon) max_lon = p->lon;
    }
    
    double lat_range = max_lat - min_lat;
    double lon_range = max_lon - min_lon;
    
    if (lat_range <= 0.0 || lon_range <= 0.0) return NULL;
    
    double scale_x = allocation.width / lon_range;
    double scale_y = allocation.height / lat_range;
    double base_scale = (scale_x < scale_y) ? scale_x : scale_y;
    base_scale *= 0.9;
    double scale = base_scale * app->zoom_factor;
    
    double center_x = allocation.width / 2.0;
    double center_y = allocation.height / 2.0;
    double map_center_x = (min_lon + max_lon) / 2.0;
    double map_center_y = (min_lat + max_lat) / 2.0;
    
    // Find closest point
    Ponto *closest = NULL;
    double min_distance = 20.0; // Maximum click distance in pixels
    
    for (size_t i = 0; i < app->grafo->num_pontos; i++) {
        Ponto *p = &app->grafo->pontos[i];
        double x = center_x + (p->lon - map_center_x) * scale + app->pan_x;
        double y = center_y - (p->lat - map_center_y) * scale + app->pan_y;
        
        double distance = sqrt((x - click_x) * (x - click_x) + (y - click_y) * (y - click_y));
        if (distance < min_distance) {
            min_distance = distance;
            closest = p;
        }
    }
    
    return closest;
}

// Callback para início do drag (botão pressionado)
gboolean on_graph_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    
    printf("DEBUG: Button press - button: %d, x: %.2f, y: %.2f, state: %d\n", 
           event->button, event->x, event->y, event->state);
    printf("DEBUG: GDK_CONTROL_MASK = %d, Ctrl pressed: %s\n", 
           GDK_CONTROL_MASK, (event->state & GDK_CONTROL_MASK) ? "YES" : "NO");
    
    if (event->button == 1) { // Botão esquerdo
        // Check if Ctrl is pressed for point selection
        if (event->state & GDK_CONTROL_MASK) {
            printf("DEBUG: Ctrl+click detected, looking for closest point...\n");
            // Point selection mode
            Ponto *clicked_point = find_closest_point(app, event->x, event->y);
            if (clicked_point) {
                printf("DEBUG: Found point %lld!\n", clicked_point->id);
                
                if (!app->has_start_point) {
                    // Select as start point
                    app->selected_start_id = clicked_point->id;
                    app->has_start_point = TRUE;
                    
                    // Update entry field
                    gchar *id_str = g_strdup_printf("%lld", clicked_point->id);
                    gtk_entry_set_text(GTK_ENTRY(app->start_entry), id_str);
                    g_free(id_str);
                    
                    printf("DEBUG: Selected as START point: %lld\n", clicked_point->id);
                    update_status(app, "Start point selected. Hold Ctrl and click again to select end point.");
                } else if (!app->has_end_point) {
                    // Select as end point
                    app->selected_end_id = clicked_point->id;
                    app->has_end_point = TRUE;
                    
                    // Update entry field
                    gchar *id_str = g_strdup_printf("%lld", clicked_point->id);
                    gtk_entry_set_text(GTK_ENTRY(app->end_entry), id_str);
                    g_free(id_str);
                    
                    printf("DEBUG: Selected as END point: %lld\n", clicked_point->id);
                    
                    update_status(app, "End point selected. Click 'Find Shortest Path' or hold Ctrl and click to select new points.");
                } else {
                    // Both points selected, reset and start over
                    app->selected_start_id = clicked_point->id;
                    app->selected_end_id = 0;
                    app->has_start_point = TRUE;
                    app->has_end_point = FALSE;
                    
                    // Update entry fields
                    gchar *id_str = g_strdup_printf("%lld", clicked_point->id);
                    gtk_entry_set_text(GTK_ENTRY(app->start_entry), id_str);
                    gtk_entry_set_text(GTK_ENTRY(app->end_entry), "");
                    g_free(id_str);
                    
                    update_status(app, "New start point selected. Hold Ctrl and click again to select end point.");
                }
                
                gtk_widget_queue_draw(widget);
                return TRUE;
            } else {
                printf("DEBUG: No point found near click location\n");
            }
        } else {
            printf("DEBUG: Normal click (no Ctrl), starting drag mode\n");
            // Normal drag mode
            app->dragging = TRUE;
            app->last_mouse_x = event->x;
            app->last_mouse_y = event->y;
        }
    }
    
    return TRUE;
}

// Callback para fim do drag (botão liberado)
gboolean on_graph_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    
    if (event->button == 1) { // Botão esquerdo
        app->dragging = FALSE;
    }
    
    return TRUE;
}

// Callback para movimento do mouse (drag)
gboolean on_graph_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    
    if (app->dragging) {
        double dx = event->x - app->last_mouse_x;
        double dy = event->y - app->last_mouse_y;
        
        app->pan_x += dx;
        app->pan_y += dy;
        
        app->last_mouse_x = event->x;
        app->last_mouse_y = event->y;
        
        gtk_widget_queue_draw(widget);
    }
    
    return TRUE;
}

// Callback para abrir arquivo OSM
void on_open_osm_clicked(GtkMenuItem *menuitem, gpointer user_data) {
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
            
            // Reset zoom and pan
            app->zoom_factor = 1.0;
            app->pan_x = 0.0;
            app->pan_y = 0.0;
            
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
    on_open_osm_clicked(NULL, user_data);
}

// Callback para encontrar caminho mais curto
void on_find_path_clicked(GtkButton *button, gpointer user_data) {
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
        
        liberar_resultado_dijkstra(resultado);
    } else {
        gchar *error_msg = g_strdup_printf("No path found between points!\n\nStart Point:\n  ID: %lld\n  Lat: %.6f\n  Lon: %.6f\n\nEnd Point:\n  ID: %lld\n  Lat: %.6f\n  Lon: %.6f\n\nThe points may be in disconnected components of the graph.\n",
                                          start_point->id, start_point->lat, start_point->lon,
                                          end_point->id, end_point->lat, end_point->lon);
        gtk_text_buffer_set_text(buffer, error_msg, -1);
        g_free(error_msg);
        update_status(app, "No path found");
        
        if (resultado) {
            liberar_resultado_dijkstra(resultado);
        }
    }
}

// Callback para limpar
void on_clear_clicked(GtkToolButton *toolbutton, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    
    gtk_entry_set_text(GTK_ENTRY(app->start_entry), "");
    gtk_entry_set_text(GTK_ENTRY(app->end_entry), "");
    
    // Clear selected points
    app->has_start_point = FALSE;
    app->has_end_point = FALSE;
    app->selected_start_id = 0;
    app->selected_end_id = 0;
    
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
    gtk_main_quit();
}

// Função para limpar dados da aplicação
void cleanup_app_data(AppData *app) {
    if (app->grafo) {
        liberar_grafo(app->grafo);
    }
    g_free(app->current_file);
}

// Callback para fechamento da janela
gboolean on_window_delete(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    cleanup_app_data(app);
    gtk_main_quit();
    return FALSE;
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

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    // Carregar UI
    GtkBuilder *builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder, "data/my_window.ui", NULL)) {
        g_printerr("Error loading UI file\n");
        return 1;
    }
    
    // Criar dados da aplicação
    AppData *app = g_malloc0(sizeof(AppData));
    
    // Inicializar variáveis de zoom e pan
    app->zoom_factor = 1.0;
    app->pan_x = 0.0;
    app->pan_y = 0.0;
    app->dragging = FALSE;
    
    // Inicializar variáveis de seleção
    app->has_start_point = FALSE;
    app->has_end_point = FALSE;
    app->selected_start_id = 0;
    app->selected_end_id = 0;
    
    // Obter widgets
    app->window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    app->file_label = GTK_WIDGET(gtk_builder_get_object(builder, "file_label"));
    app->stats_label = GTK_WIDGET(gtk_builder_get_object(builder, "stats_label"));
    app->start_entry = GTK_WIDGET(gtk_builder_get_object(builder, "start_entry"));
    app->end_entry = GTK_WIDGET(gtk_builder_get_object(builder, "end_entry"));
    app->results_text = GTK_WIDGET(gtk_builder_get_object(builder, "results_text"));
    app->graph_area = GTK_WIDGET(gtk_builder_get_object(builder, "graph_area"));
    app->statusbar = GTK_WIDGET(gtk_builder_get_object(builder, "statusbar"));
    
    // Configurar status bar
    app->status_context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(app->statusbar), "main");
    
    // Conectar sinais
    g_signal_connect(app->window, "delete-event", G_CALLBACK(on_window_delete), app);
    g_signal_connect(gtk_builder_get_object(builder, "open_osm_item"), "activate", G_CALLBACK(on_open_osm_clicked), app);
    g_signal_connect(gtk_builder_get_object(builder, "quit_item"), "activate", G_CALLBACK(on_quit_clicked), app);
    g_signal_connect(gtk_builder_get_object(builder, "load_button"), "clicked", G_CALLBACK(on_load_osm_clicked), app);
    g_signal_connect(gtk_builder_get_object(builder, "clear_button"), "clicked", G_CALLBACK(on_clear_clicked), app);
    g_signal_connect(gtk_builder_get_object(builder, "find_path_button"), "clicked", G_CALLBACK(on_find_path_clicked), app);
    g_signal_connect(app->graph_area, "draw", G_CALLBACK(on_graph_draw), app);
    g_signal_connect(app->graph_area, "button-press-event", G_CALLBACK(on_graph_button_press), app);
    g_signal_connect(app->graph_area, "button-release-event", G_CALLBACK(on_graph_button_release), app);
    g_signal_connect(app->graph_area, "motion-notify-event", G_CALLBACK(on_graph_motion_notify), app);
    g_signal_connect(app->graph_area, "scroll-event", G_CALLBACK(on_graph_scroll), app);
    
    // Habilitar eventos necessários no graph_area
    gtk_widget_add_events(app->graph_area, 
                         GDK_BUTTON_PRESS_MASK | 
                         GDK_BUTTON_RELEASE_MASK | 
                         GDK_POINTER_MOTION_MASK | 
                         GDK_SCROLL_MASK);
    
    // Inicializar interface
    update_file_info(app);
    update_status(app, "Ready");
    
    // Mostrar janela
    gtk_widget_show_all(app->window);
    
    // Executar loop principal
    gtk_main();
    
    // Limpar
    g_free(app);
    g_object_unref(builder);
    
    return 0;
}