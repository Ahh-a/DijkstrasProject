#include "interaction.h"
#include "ui_helpers.h"
#include <math.h>

// Callback para zoom com scroll do mouse
gboolean on_graph_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    
    if (!app->grafo) return FALSE;
    
    double zoom_factor = 1.1;
    double old_zoom = app->zoom_factor;
    
    if (event->direction == GDK_SCROLL_UP) {
        app->zoom_factor *= zoom_factor;
    } else if (event->direction == GDK_SCROLL_DOWN) {
        app->zoom_factor /= zoom_factor;
    }
    
    // Limitar o zoom
    if (app->zoom_factor < 0.1) app->zoom_factor = 0.1;
    if (app->zoom_factor > 50.0) app->zoom_factor = 50.0;
    
    // Ajustar pan para manter o mouse no centro do zoom
    double zoom_ratio = app->zoom_factor / old_zoom;
    if (zoom_ratio != 1.0) {
        GtkAllocation allocation;
        gtk_widget_get_allocation(widget, &allocation);
        
        // Posição do mouse em coordenadas da tela
        double mouse_x = event->x;
        double mouse_y = event->y;
        
        // Posição do mouse em coordenadas do mundo (antes do zoom)
        double world_x = (mouse_x - allocation.width/2 - app->pan_x) / old_zoom;
        double world_y = (mouse_y - allocation.height/2 - app->pan_y) / old_zoom;
        
        // Ajustar pan para manter o ponto do mundo sob o mouse
        app->pan_x = mouse_x - allocation.width/2 - world_x * app->zoom_factor;
        app->pan_y = mouse_y - allocation.height/2 - world_y * app->zoom_factor;
    }
    
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

// Função para converter coordenadas de tela para lat/lon
void screen_to_latlon(AppData *app, double screen_x, double screen_y, double *lat, double *lon) {
    if (!app->grafo) return;
    
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
    
    if (lat_range <= 0.0 || lon_range <= 0.0) return;
    
    double scale_x = allocation.width / lon_range;
    double scale_y = allocation.height / lat_range;
    double base_scale = (scale_x < scale_y) ? scale_x : scale_y;
    base_scale *= 0.9;
    double scale = base_scale * app->zoom_factor;
    
    double center_x = allocation.width / 2.0;
    double center_y = allocation.height / 2.0;
    double map_center_x = (min_lon + max_lon) / 2.0;
    double map_center_y = (min_lat + max_lat) / 2.0;
    
    // Convert back from screen coordinates
    *lon = ((screen_x - center_x - app->pan_x) / scale) + map_center_x;
    *lat = -((screen_y - center_y - app->pan_y) / scale) + map_center_y;
}

// Função para lidar com cliques no modo de edição
gboolean handle_edit_click(AppData *app, double click_x, double click_y, GtkWidget *widget) {
    if (!app->grafo) return FALSE;
    
    switch (app->edit_state.current_mode) {
        case EDIT_MODE_CREATE: {
            // Converter coordenadas de tela para lat/lon
            double lat, lon;
            screen_to_latlon(app, click_x, click_y, &lat, &lon);
            
            // Criar novo nó
            if (create_node_at_position(app->grafo, &app->edit_state, lat, lon)) {
                update_node_ids(&app->edit_state, app->grafo);
                mark_graph_as_modified(app->grafo);
                update_status(app, "New node created. Click to create more nodes.");
                gtk_widget_queue_draw(widget);
                return TRUE;
            }
            break;
        }
        
        case EDIT_MODE_DELETE: {
            // Encontrar nó mais próximo
            Ponto *clicked_point = find_closest_point(app, click_x, click_y);
            if (clicked_point) {
                if (can_delete_node(app->grafo, clicked_point->id)) {
                    if (delete_node(app->grafo, &app->edit_state, clicked_point->id)) {
                        mark_graph_as_modified(app->grafo);
                        update_status(app, "Node deleted. Click to delete more nodes.");
                        gtk_widget_queue_draw(widget);
                        return TRUE;
                    }
                }
            }
            break;
        }
        
        case EDIT_MODE_CONNECT: {
            // Encontrar nó mais próximo
            Ponto *clicked_point = find_closest_point(app, click_x, click_y);
            if (clicked_point) {
                if (!app->edit_state.is_connecting) {
                    // Primeiro clique - selecionar nó de origem
                    app->edit_state.is_connecting = TRUE;
                    app->edit_state.connecting_from_id = clicked_point->id;
                    update_status(app, "Click another node to connect to this one.");
                    gtk_widget_queue_draw(widget);
                    return TRUE;
                } else {
                    // Segundo clique - conectar aos nós
                    if (clicked_point->id != app->edit_state.connecting_from_id) {
                        if (connect_nodes(app->grafo, &app->edit_state, 
                                        app->edit_state.connecting_from_id, clicked_point->id)) {
                            mark_graph_as_modified(app->grafo);
                            update_status(app, "Nodes connected. Click a node to start new connection.");
                        } else {
                            update_status(app, "Failed to connect nodes. Click a node to start new connection.");
                        }
                    } else {
                        update_status(app, "Cannot connect node to itself. Click a different node.");
                    }
                    
                    // Reset connection state
                    app->edit_state.is_connecting = FALSE;
                    app->edit_state.connecting_from_id = 0;
                    gtk_widget_queue_draw(widget);
                    return TRUE;
                }
            }
            break;
        }
        
        default:
            break;
    }
    
    return FALSE;
}

// Callback para cliques no botão do mouse na área do grafo
gboolean on_graph_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    
    if (event->button == 1) { // Botão esquerdo
        // Verificar se está em modo de edição
        if (app->edit_state.current_mode != EDIT_MODE_NONE) {
            if (handle_edit_click(app, event->x, event->y, widget)) {
                return TRUE;
            }
        }
        
        // Verificar se Ctrl está pressionado para seleção de pontos
        if (event->state & GDK_CONTROL_MASK) {
            Ponto *clicked_point = find_closest_point(app, event->x, event->y);
            if (clicked_point) {
                if (!app->has_start_point) {
                    // Selecionar como ponto inicial
                    app->selected_start_id = clicked_point->id;
                    app->has_start_point = TRUE;
                    
                    // Atualizar entry
                    gchar *id_str = g_strdup_printf("%lld", clicked_point->id);
                    gtk_entry_set_text(GTK_ENTRY(app->start_entry), id_str);
                    g_free(id_str);
                    
                    update_status(app, "Start point selected. Hold Ctrl and click another point to select end point.");
                } else if (!app->has_end_point) {
                    // Selecionar como ponto final
                    app->selected_end_id = clicked_point->id;
                    app->has_end_point = TRUE;
                    
                    // Atualizar entry
                    gchar *id_str = g_strdup_printf("%lld", clicked_point->id);
                    gtk_entry_set_text(GTK_ENTRY(app->end_entry), id_str);
                    g_free(id_str);
                    
                    update_status(app, "End point selected. Click 'Find Shortest Path' to calculate route.");
                } else {
                    // Reset e selecionar novo ponto inicial
                    app->selected_start_id = clicked_point->id;
                    app->selected_end_id = 0;
                    app->has_start_point = TRUE;
                    app->has_end_point = FALSE;
                    
                    // Limpar caminho mais curto
                    if (app->shortest_path) {
                        free(app->shortest_path);
                        app->shortest_path = NULL;
                    }
                    app->has_shortest_path = FALSE;
                    app->shortest_path_length = 0;
                    
                    // Atualizar entries
                    gchar *id_str = g_strdup_printf("%lld", clicked_point->id);
                    gtk_entry_set_text(GTK_ENTRY(app->start_entry), id_str);
                    gtk_entry_set_text(GTK_ENTRY(app->end_entry), "");
                    g_free(id_str);
                    
                    update_status(app, "New start point selected. Hold Ctrl and click another point to select end point.");
                }
                
                gtk_widget_queue_draw(widget);
                return TRUE;
            }
        } else {
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
    (void)widget; // Suppress unused parameter warning
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
