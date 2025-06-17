#include "drawing.h"
#include "ui_helpers.h"
#include <math.h>

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
        cairo_set_line_width(cr, 1.2 / app->zoom_factor); // Era 0.8, agora 1.2
        
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
                    
                    // Check if this edge is part of the shortest path
                    gboolean is_shortest_path_edge = is_edge_in_shortest_path(app, a->origem, a->destino);
                    
                    if (is_shortest_path_edge) {
                        // Draw shortest path edges in red with thicker line
                        cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.9); // Red
                        cairo_set_line_width(cr, 3.0 / app->zoom_factor);
                    } else {
                        // Different colors for directional vs bidirectional edges
                        if (a->is_bidirectional) {
                            // Bidirectional edges in gray
                            cairo_set_source_rgba(cr, 0.4, 0.4, 0.4, 0.6); // Gray
                        } else {
                            // Directional edges in blue
                            cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 0.7); // Blue
                        }
                        // Linhas ~186-194 - Aumentar espessura específica por tipo
                        if (a->is_bidirectional) {
                            cairo_set_line_width(cr, 1.2 / app->zoom_factor); // Era 0.8, agora 1.2 - Bidirectional streets
                        } else {
                            cairo_set_line_width(cr, 1.6 / app->zoom_factor); // Era 1.2, agora 1.6 - Oneway streets - slightly thicker
                        }
                    }
                    
                    cairo_move_to(cr, x1, y1);
                    cairo_line_to(cr, x2, y2);
                    cairo_stroke(cr);
                    
                    // Draw arrow for directional edges (not bidirectional)
                    if (!a->is_bidirectional && !is_shortest_path_edge) {
                        // Calculate arrow position and direction
                        double dx = x2 - x1;
                        double dy = y2 - y1;
                        double length = sqrt(dx*dx + dy*dy);
                        
                        if (length > 0) {
                            // Normalize direction vector
                            dx /= length;
                            dy /= length;
                            
                            // Arrow position (75% along the line)
                            double arrow_x = x1 + dx * length * 0.75;
                            double arrow_y = y1 + dy * length * 0.75;
                            
                            // Arrow size based on zoom - increased for better visibility
                            double arrow_size = 18.0 / app->zoom_factor;
                            if (arrow_size < 10.0) arrow_size = 10.0;
                            if (arrow_size > 30.0) arrow_size = 30.0;
                            
                            // Draw arrowhead
                            cairo_save(cr);
                            cairo_translate(cr, arrow_x, arrow_y);
                            cairo_rotate(cr, atan2(dy, dx));
                            
                            cairo_move_to(cr, 0, 0);
                            cairo_line_to(cr, -arrow_size, -arrow_size/2);
                            cairo_line_to(cr, -arrow_size, arrow_size/2);
                            cairo_close_path(cr);
                            cairo_fill(cr);
                            
                            cairo_restore(cr);
                        }
                    }
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
                double point_radius = 4.0 / app->zoom_factor;
                if (point_radius < 2.0) point_radius = 2.0;
                if (point_radius > 6.0) point_radius = 6.0;
                
                // Check if this point is selected
                gboolean is_start = (app->has_start_point && p->id == app->selected_start_id);
                gboolean is_end = (app->has_end_point && p->id == app->selected_end_id);
                gboolean is_edit_selected = (app->edit_state.has_selected_node && p->id == app->edit_state.selected_node_id);
                gboolean is_connecting_from = (app->edit_state.is_connecting && p->id == app->edit_state.connecting_from_id);
                
                // Determinar cor baseada no estado
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
                } else if (is_connecting_from) {
                    // Connection source - blue
                    cairo_set_source_rgb(cr, 0.2, 0.2, 0.8);
                    cairo_arc(cr, x, y, point_radius + 2, 0, 2 * M_PI);
                    cairo_fill(cr);
                } else if (is_edit_selected) {
                    // Edit selected - orange
                    cairo_set_source_rgb(cr, 1.0, 0.5, 0.0);
                    cairo_arc(cr, x, y, point_radius + 2, 0, 2 * M_PI);
                    cairo_fill(cr);
                } else {
                    // Normal point - color depends on edit mode
                    switch (app->edit_state.current_mode) {
                        case EDIT_MODE_CREATE:
                            cairo_set_source_rgb(cr, 0.6, 0.8, 0.6); // Light green
                            break;
                        case EDIT_MODE_DELETE:
                            cairo_set_source_rgb(cr, 0.8, 0.6, 0.6); // Light red
                            break;
                        case EDIT_MODE_CONNECT:
                            cairo_set_source_rgb(cr, 0.6, 0.6, 0.8); // Light blue
                            break;
                        default:
                            cairo_set_source_rgb(cr, 0.8, 0.2, 0.8); // Purple (normal)
                            break;
                    }
                    cairo_arc(cr, x, y, point_radius, 0, 2 * M_PI);
                    cairo_fill(cr);
                }
            }
        }
    }
    
    return TRUE;
}

// Função para renderizar o grafo em uma superfície off-screen e salvar como PNG
void render_graph_to_png(AppData *app, const char *filename, int width, int height) {
    if (!app->grafo) {
        g_warning("No graph data to export");
        return;
    }
    
    // Criar superfície PNG
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surface);
    
    // Background branco
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    // Calcular bounds do grafo
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
        // Calcular escala para caber na imagem
        double scale_x = width / lon_range;
        double scale_y = height / lat_range;
        double scale = (scale_x < scale_y) ? scale_x : scale_y;
        scale *= 0.9; // Margem
        
        // Aplicar zoom atual da aplicação
        scale *= app->zoom_factor;
        
        // Centro da imagem
        double center_x = width / 2.0;
        double center_y = height / 2.0;
        double map_center_x = (min_lon + max_lon) / 2.0;
        double map_center_y = (min_lat + max_lat) / 2.0;
        
        // Desenhar arestas primeiro
        for (size_t i = 0; i < app->grafo->num_arestas; i++) {
            Aresta *a = &app->grafo->arestas[i];
            
            // Encontrar pontos origem e destino
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
                
                // Verificar se esta aresta faz parte do caminho mais curto
                gboolean is_shortest_path_edge = is_edge_in_shortest_path(app, a->origem, a->destino);
                
                if (is_shortest_path_edge) {
                    // Caminho mais curto em vermelho e mais grosso
                    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.9);
                    cairo_set_line_width(cr, 3.0 / app->zoom_factor);
                } else {
                    // Cores diferentes para bidirecionais vs direcionais
                    if (a->is_bidirectional) {
                        cairo_set_source_rgba(cr, 0.4, 0.4, 0.4, 0.6); // Cinza
                        cairo_set_line_width(cr, 1.2 / app->zoom_factor);
                    } else {
                        cairo_set_source_rgba(cr, 0.2, 0.2, 0.8, 0.7); // Azul
                        cairo_set_line_width(cr, 1.6 / app->zoom_factor);
                    }
                }
                
                cairo_move_to(cr, x1, y1);
                cairo_line_to(cr, x2, y2);
                cairo_stroke(cr);
                
                // Desenhar seta para arestas direcionais
                if (!a->is_bidirectional && !is_shortest_path_edge) {
                    double dx = x2 - x1;
                    double dy = y2 - y1;
                    double length = sqrt(dx*dx + dy*dy);
                    
                    if (length > 0) {
                        dx /= length;
                        dy /= length;
                        
                        // Posição da seta (75% ao longo da linha)
                        double arrow_x = x1 + dx * length * 0.75;
                        double arrow_y = y1 + dy * length * 0.75;
                        
                        // Tamanho da seta
                        double arrow_size = 18.0 / app->zoom_factor;
                        if (arrow_size < 10.0) arrow_size = 10.0;
                        if (arrow_size > 30.0) arrow_size = 30.0;
                        
                        // Desenhar cabeça da seta
                        cairo_save(cr);
                        cairo_translate(cr, arrow_x, arrow_y);
                        cairo_rotate(cr, atan2(dy, dx));
                        
                        cairo_move_to(cr, 0, 0);
                        cairo_line_to(cr, -arrow_size, -arrow_size/2);
                        cairo_line_to(cr, -arrow_size, arrow_size/2);
                        cairo_close_path(cr);
                        cairo_fill(cr);
                        
                        cairo_restore(cr);
                    }
                }
            }
        }
        
        // Desenhar pontos (por cima das arestas)
        for (size_t i = 0; i < app->grafo->num_pontos; i++) {
            Ponto *p = &app->grafo->pontos[i];
            double x = center_x + (p->lon - map_center_x) * scale + app->pan_x;
            double y = center_y - (p->lat - map_center_y) * scale + app->pan_y;
            
            double point_radius = 4.0 / app->zoom_factor;
            if (point_radius < 2.0) point_radius = 2.0;
            if (point_radius > 6.0) point_radius = 6.0;
            
            // Verificar se este ponto está selecionado
            gboolean is_start = (app->has_start_point && p->id == app->selected_start_id);
            gboolean is_end = (app->has_end_point && p->id == app->selected_end_id);
            
            if (is_start) {
                // Ponto inicial - verde
                cairo_set_source_rgb(cr, 0.2, 0.8, 0.2);
                cairo_arc(cr, x, y, point_radius + 2, 0, 2 * M_PI);
                cairo_fill(cr);
            } else if (is_end) {
                // Ponto final - vermelho
                cairo_set_source_rgb(cr, 0.8, 0.2, 0.2);
                cairo_arc(cr, x, y, point_radius + 2, 0, 2 * M_PI);
                cairo_fill(cr);
            } else {
                // Ponto normal - roxo
                cairo_set_source_rgb(cr, 0.8, 0.2, 0.8);
                cairo_arc(cr, x, y, point_radius, 0, 2 * M_PI);
                cairo_fill(cr);
            }
        }
    }
    
    // Salvar a imagem PNG
    cairo_surface_write_to_png(surface, filename);
    
    // Limpar recursos
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
