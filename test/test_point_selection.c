#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include "osm_reader.h"
#include "graph.h"
#include "dijkstra.h"

// Estrutura simples para teste
typedef struct {
    GtkWidget *window;
    GtkWidget *graph_area;
    Grafo *grafo;
    double zoom_factor;
    double pan_x, pan_y;
    double last_mouse_x, last_mouse_y;
    gboolean dragging;
    long long selected_start_id;
    long long selected_end_id;
    gboolean has_start_point;
    gboolean has_end_point;
} TestAppData;

// Função para encontrar ponto mais próximo
Ponto* find_closest_point_test(TestAppData *app, double click_x, double click_y) {
    if (!app->grafo) {
        printf("DEBUG: No grafo loaded\n");
        return NULL;
    }
    
    printf("DEBUG: Finding closest point to (%.2f, %.2f)\n", click_x, click_y);
    printf("DEBUG: Graph has %zu points\n", app->grafo->num_pontos);
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(app->graph_area, &allocation);
    
    printf("DEBUG: Widget allocation: %dx%d\n", allocation.width, allocation.height);
    
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
    
    printf("DEBUG: Bounds: lat [%.6f, %.6f], lon [%.6f, %.6f]\n", min_lat, max_lat, min_lon, max_lon);
    
    double lat_range = max_lat - min_lat;
    double lon_range = max_lon - min_lon;
    
    if (lat_range <= 0.0 || lon_range <= 0.0) {
        printf("DEBUG: Invalid range\n");
        return NULL;
    }
    
    double scale_x = allocation.width / lon_range;
    double scale_y = allocation.height / lat_range;
    double base_scale = (scale_x < scale_y) ? scale_x : scale_y;
    base_scale *= 0.9;
    double scale = base_scale * app->zoom_factor;
    
    printf("DEBUG: Scale: %.2f (base: %.2f, zoom: %.2f)\n", scale, base_scale, app->zoom_factor);
    
    double center_x = allocation.width / 2.0;
    double center_y = allocation.height / 2.0;
    double map_center_x = (min_lon + max_lon) / 2.0;
    double map_center_y = (min_lat + max_lat) / 2.0;
    
    // Find closest point
    Ponto *closest = NULL;
    double min_distance = 20.0; // Maximum click distance in pixels
    int checked_points = 0;
    
    for (size_t i = 0; i < app->grafo->num_pontos && i < 10; i++) { // Check first 10 for debug
        Ponto *p = &app->grafo->pontos[i];
        double x = center_x + (p->lon - map_center_x) * scale + app->pan_x;
        double y = center_y - (p->lat - map_center_y) * scale + app->pan_y;
        
        double distance = sqrt((x - click_x) * (x - click_x) + (y - click_y) * (y - click_y));
        
        printf("DEBUG: Point %lld at screen (%.2f, %.2f), distance: %.2f\n", p->id, x, y, distance);
        
        if (distance < min_distance) {
            min_distance = distance;
            closest = p;
            printf("DEBUG: New closest point: %lld (distance: %.2f)\n", p->id, distance);
        }
        checked_points++;
    }
    
    printf("DEBUG: Checked %d points, closest: %s\n", checked_points, closest ? "found" : "none");
    
    return closest;
}

// Callback de botão
gboolean on_button_press_test(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    TestAppData *app = (TestAppData *)user_data;
    
    printf("DEBUG: Button press event - button: %d, x: %.2f, y: %.2f\n", event->button, event->x, event->y);
    printf("DEBUG: Event state: %d (GDK_CONTROL_MASK = %d)\n", event->state, GDK_CONTROL_MASK);
    
    if (event->button == 1) { // Left button
        if (event->state & GDK_CONTROL_MASK) {
            printf("DEBUG: Ctrl+click detected!\n");
            Ponto *clicked_point = find_closest_point_test(app, event->x, event->y);
            if (clicked_point) {
                printf("SUCCESS: Found point %lld!\n", clicked_point->id);
                
                if (!app->has_start_point) {
                    app->selected_start_id = clicked_point->id;
                    app->has_start_point = TRUE;
                    printf("Selected as START point: %lld\n", clicked_point->id);
                } else if (!app->has_end_point) {
                    app->selected_end_id = clicked_point->id;
                    app->has_end_point = TRUE;
                    printf("Selected as END point: %lld\n", clicked_point->id);
                } else {
                    // Reset
                    app->selected_start_id = clicked_point->id;
                    app->selected_end_id = 0;
                    app->has_start_point = TRUE;
                    app->has_end_point = FALSE;
                    printf("Reset - new START point: %lld\n", clicked_point->id);
                }
                
                gtk_widget_queue_draw(widget);
                return TRUE;
            } else {
                printf("DEBUG: No point found near click\n");
            }
        } else {
            printf("DEBUG: Normal click (no Ctrl)\n");
            app->dragging = TRUE;
            app->last_mouse_x = event->x;
            app->last_mouse_y = event->y;
        }
    }
    
    return TRUE;
}

// Callback de desenho simples
gboolean on_draw_test(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    TestAppData *app = (TestAppData *)user_data;
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // White background
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    if (!app->grafo) {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 10, 30);
        cairo_show_text(cr, "No graph loaded. Load an OSM file first.");
        return FALSE;
    }
    
    // Simple drawing of points
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
        double scale_x = allocation.width / lon_range;
        double scale_y = allocation.height / lat_range;
        double base_scale = (scale_x < scale_y) ? scale_x : scale_y;
        base_scale *= 0.9;
        double scale = base_scale * app->zoom_factor;
        
        double center_x = allocation.width / 2.0;
        double center_y = allocation.height / 2.0;
        double map_center_x = (min_lon + max_lon) / 2.0;
        double map_center_y = (min_lat + max_lat) / 2.0;
        
        // Draw points
        for (size_t i = 0; i < app->grafo->num_pontos; i++) {
            Ponto *p = &app->grafo->pontos[i];
            double x = center_x + (p->lon - map_center_x) * scale + app->pan_x;
            double y = center_y - (p->lat - map_center_y) * scale + app->pan_y;
            
            // Different colors for selected points
            if (app->has_start_point && p->id == app->selected_start_id) {
                cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); // Green for start
                cairo_arc(cr, x, y, 4, 0, 2 * M_PI);
            } else if (app->has_end_point && p->id == app->selected_end_id) {
                cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); // Red for end  
                cairo_arc(cr, x, y, 4, 0, 2 * M_PI);
            } else {
                cairo_set_source_rgb(cr, 0.5, 0.0, 1.0); // Purple for normal
                cairo_arc(cr, x, y, 2, 0, 2 * M_PI);
            }
            cairo_fill(cr);
        }
    }
    
    // Show status
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, 10, 20);
    if (app->has_start_point && app->has_end_point) {
        char status[200];
        snprintf(status, sizeof(status), "Start: %lld, End: %lld (Ctrl+click to change)", 
                app->selected_start_id, app->selected_end_id);
        cairo_show_text(cr, status);
    } else if (app->has_start_point) {
        char status[200];
        snprintf(status, sizeof(status), "Start: %lld (Ctrl+click to select end point)", 
                app->selected_start_id);
        cairo_show_text(cr, status);
    } else {
        cairo_show_text(cr, "Ctrl+click to select start point");
    }
    
    return FALSE;
}

// Callback de tecla para carregar arquivo
gboolean on_key_press_test(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    TestAppData *app = (TestAppData *)user_data;
    
    if (event->keyval == GDK_KEY_o || event->keyval == GDK_KEY_O) {
        printf("Loading test file...\n");
        
        if (app->grafo) {
            liberar_grafo(app->grafo);
        }
        
        app->grafo = ler_osm("/home/vitorette/Documentos/3semestre/EAD2/DijkstrasProject/casaPrimo.osm");
        
        if (app->grafo) {
            printf("Loaded %zu points, %zu edges\n", app->grafo->num_pontos, app->grafo->num_arestas);
        } else {
            printf("Failed to load OSM file\n");
        }
        
        // Reset selection
        app->has_start_point = FALSE;
        app->has_end_point = FALSE;
        app->selected_start_id = 0;
        app->selected_end_id = 0;
        
        gtk_widget_queue_draw(app->graph_area);
        return TRUE;
    }
    
    return FALSE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    TestAppData *app = g_new0(TestAppData, 1);
    app->zoom_factor = 1.0;
    app->pan_x = 0.0;
    app->pan_y = 0.0;
    app->dragging = FALSE;
    app->has_start_point = FALSE;
    app->has_end_point = FALSE;
    
    // Create window
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "Point Selection Test");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 800, 600);
    
    // Create drawing area
    app->graph_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(app->window), app->graph_area);
    
    // Connect signals
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(app->graph_area, "draw", G_CALLBACK(on_draw_test), app);
    g_signal_connect(app->graph_area, "button-press-event", G_CALLBACK(on_button_press_test), app);
    g_signal_connect(app->window, "key-press-event", G_CALLBACK(on_key_press_test), app);
    
    // Enable events
    gtk_widget_add_events(app->graph_area, 
                         GDK_BUTTON_PRESS_MASK | 
                         GDK_BUTTON_RELEASE_MASK | 
                         GDK_POINTER_MOTION_MASK);
    
    gtk_widget_set_can_focus(app->window, TRUE);
    
    gtk_widget_show_all(app->window);
    
    printf("Test application started.\n");
    printf("Press 'O' to load OSM file, then Ctrl+click to select points\n");
    
    gtk_main();
    
    if (app->grafo) {
        liberar_grafo(app->grafo);
    }
    g_free(app);
    
    return 0;
}
