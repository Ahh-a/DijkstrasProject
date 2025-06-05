#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include "osm_reader.h"
#include "graph.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    GtkWidget *window;
    GtkWidget *graph_area;
    GtkWidget *status_label;
    Grafo *grafo;
    double zoom_factor;
    double pan_x, pan_y;
    long long selected_start_id;
    long long selected_end_id;
    gboolean has_start_point;
    gboolean has_end_point;
} SimpleApp;

Ponto* find_closest_point_simple(SimpleApp *app, double click_x, double click_y) {
    if (!app->grafo) {
        printf("DEBUG: Nenhum grafo carregado\n");
        return NULL;
    }
    
    printf("DEBUG: Procurando ponto próximo a (%.2f, %.2f)\n", click_x, click_y);
    printf("DEBUG: Grafo tem %zu pontos\n", app->grafo->num_pontos);
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(app->graph_area, &allocation);
    
    // Calcular limites
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
    
    if (lat_range <= 0.0 || lon_range <= 0.0) {
        printf("DEBUG: Range inválido\n");
        return NULL;
    }
    
    double scale_x = allocation.width / lon_range;
    double scale_y = allocation.height / lat_range;
    double base_scale = (scale_x < scale_y) ? scale_x : scale_y;
    base_scale *= 0.9;
    double scale = base_scale * app->zoom_factor;
    
    double center_x = allocation.width / 2.0;
    double center_y = allocation.height / 2.0;
    double map_center_x = (min_lon + max_lon) / 2.0;
    double map_center_y = (min_lat + max_lat) / 2.0;
    
    printf("DEBUG: Scale: %.2f, Center: (%.2f, %.2f)\n", scale, center_x, center_y);
    
    Ponto *closest = NULL;
    double min_distance = 20.0; // Distância máxima em pixels
    
    for (size_t i = 0; i < app->grafo->num_pontos; i++) {
        Ponto *p = &app->grafo->pontos[i];
        double x = center_x + (p->lon - map_center_x) * scale + app->pan_x;
        double y = center_y - (p->lat - map_center_y) * scale + app->pan_y;
        
        double distance = sqrt((x - click_x) * (x - click_x) + (y - click_y) * (y - click_y));
        
        if (i < 5) { // Debug primeiros 5 pontos
            printf("DEBUG: Ponto %lld em tela (%.2f, %.2f), distância: %.2f\n", p->id, x, y, distance);
        }
        
        if (distance < min_distance) {
            min_distance = distance;
            closest = p;
            printf("DEBUG: Novo ponto mais próximo: %lld (distância: %.2f)\n", p->id, distance);
        }
    }
    
    return closest;
}

gboolean on_button_press_simple(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    SimpleApp *app = (SimpleApp *)user_data;
    
    printf("\n=== EVENTO DE CLIQUE ===\n");
    printf("Botão: %d\n", event->button);
    printf("Posição: (%.2f, %.2f)\n", event->x, event->y);
    printf("Estado: %d\n", event->state);
    printf("GDK_CONTROL_MASK: %d\n", GDK_CONTROL_MASK);
    printf("Ctrl pressionado: %s\n", (event->state & GDK_CONTROL_MASK) ? "SIM" : "NÃO");
    
    if (event->button == 1 && (event->state & GDK_CONTROL_MASK)) {
        printf(">>> CTRL+CLIQUE DETECTADO! <<<\n");
        
        Ponto *clicked_point = find_closest_point_simple(app, event->x, event->y);
        if (clicked_point) {
            printf(">>> PONTO ENCONTRADO: %lld <<<\n", clicked_point->id);
            
            char status_text[200];
            if (!app->has_start_point) {
                app->selected_start_id = clicked_point->id;
                app->has_start_point = TRUE;
                snprintf(status_text, sizeof(status_text), 
                        "INÍCIO selecionado: %lld - Ctrl+clique para selecionar FIM", 
                        clicked_point->id);
                printf("SELECIONADO COMO INÍCIO: %lld\n", clicked_point->id);
            } else if (!app->has_end_point) {
                app->selected_end_id = clicked_point->id;
                app->has_end_point = TRUE;
                snprintf(status_text, sizeof(status_text), 
                        "INÍCIO: %lld, FIM: %lld - Ambos selecionados!", 
                        app->selected_start_id, clicked_point->id);
                printf("SELECIONADO COMO FIM: %lld\n", clicked_point->id);
            } else {
                // Reset
                app->selected_start_id = clicked_point->id;
                app->selected_end_id = 0;
                app->has_start_point = TRUE;
                app->has_end_point = FALSE;
                snprintf(status_text, sizeof(status_text), 
                        "Novo INÍCIO: %lld - Ctrl+clique para selecionar FIM", 
                        clicked_point->id);
                printf("RESETADO - NOVO INÍCIO: %lld\n", clicked_point->id);
            }
            
            gtk_label_set_text(GTK_LABEL(app->status_label), status_text);
            gtk_widget_queue_draw(widget);
            return TRUE;
        } else {
            printf(">>> NENHUM PONTO ENCONTRADO PRÓXIMO AO CLIQUE <<<\n");
            gtk_label_set_text(GTK_LABEL(app->status_label), "Nenhum ponto encontrado próximo ao clique");
        }
    } else if (event->button == 1) {
        printf("Clique normal (sem Ctrl)\n");
        gtk_label_set_text(GTK_LABEL(app->status_label), "Clique normal - use Ctrl+clique para selecionar pontos");
    }
    
    printf("========================\n\n");
    return TRUE;
}

gboolean on_draw_simple(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    SimpleApp *app = (SimpleApp *)user_data;
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // Fundo branco
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    if (!app->grafo) {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 10, 30);
        cairo_show_text(cr, "Pressione 'O' para carregar arquivo OSM");
        return FALSE;
    }
    
    // Desenhar pontos
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
        
        // Desenhar arestas primeiro
        cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 0.5);
        cairo_set_line_width(cr, 1.0);
        
        for (size_t i = 0; i < app->grafo->num_arestas; i++) {
            Aresta *a = &app->grafo->arestas[i];
            
            Ponto *origem = NULL, *destino = NULL;
            for (size_t j = 0; j < app->grafo->num_pontos; j++) {
                if (app->grafo->pontos[j].id == a->origem) origem = &app->grafo->pontos[j];
                if (app->grafo->pontos[j].id == a->destino) destino = &app->grafo->pontos[j];
                if (origem && destino) break;
            }
            
            if (origem && destino) {
                double x1 = center_x + (origem->lon - map_center_x) * scale + app->pan_x;
                double y1 = center_y - (origem->lat - map_center_y) * scale + app->pan_y;
                double x2 = center_x + (destino->lon - map_center_x) * scale + app->pan_x;
                double y2 = center_y - (destino->lat - map_center_y) * scale + app->pan_y;
                
                cairo_move_to(cr, x1, y1);
                cairo_line_to(cr, x2, y2);
                cairo_stroke(cr);
            }
        }
        
        // Desenhar pontos
        for (size_t i = 0; i < app->grafo->num_pontos; i++) {
            Ponto *p = &app->grafo->pontos[i];
            double x = center_x + (p->lon - map_center_x) * scale + app->pan_x;
            double y = center_y - (p->lat - map_center_y) * scale + app->pan_y;
            
            // Cores diferentes para pontos selecionados
            if (app->has_start_point && p->id == app->selected_start_id) {
                cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); // Verde para início
                cairo_arc(cr, x, y, 6, 0, 2 * M_PI);
            } else if (app->has_end_point && p->id == app->selected_end_id) {
                cairo_set_source_rgb(cr, 1.0, 0.0, 0.0); // Vermelho para fim
                cairo_arc(cr, x, y, 6, 0, 2 * M_PI);
            } else {
                cairo_set_source_rgb(cr, 0.2, 0.2, 0.8); // Azul para normal
                cairo_arc(cr, x, y, 3, 0, 2 * M_PI);
            }
            cairo_fill(cr);
        }
    }
    
    // Informações no topo
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, 10, 20);
    char info[200];
    snprintf(info, sizeof(info), "Pontos: %zu | Arestas: %zu | Ctrl+clique para selecionar", 
             app->grafo->num_pontos, app->grafo->num_arestas);
    cairo_show_text(cr, info);
    
    return FALSE;
}

gboolean on_key_press_simple(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    SimpleApp *app = (SimpleApp *)user_data;
    
    if (event->keyval == GDK_KEY_o || event->keyval == GDK_KEY_O) {
        printf("\n=== CARREGANDO ARQUIVO OSM ===\n");
        
        if (app->grafo) {
            liberar_grafo(app->grafo);
        }
        
        const char *filename = "/home/vitorette/Documentos/3semestre/EAD2/DijkstrasProject/casaPrimo.osm";
        printf("Carregando: %s\n", filename);
        
        app->grafo = ler_osm(filename);
        
        if (app->grafo) {
            printf("✓ Arquivo carregado com sucesso!\n");
            printf("✓ Pontos: %zu\n", app->grafo->num_pontos);
            printf("✓ Arestas: %zu\n", app->grafo->num_arestas);
            
            char status_text[200];
            snprintf(status_text, sizeof(status_text), 
                    "Arquivo carregado: %zu pontos, %zu arestas - Ctrl+clique para selecionar", 
                    app->grafo->num_pontos, app->grafo->num_arestas);
            gtk_label_set_text(GTK_LABEL(app->status_label), status_text);
        } else {
            printf("✗ Erro ao carregar arquivo!\n");
            gtk_label_set_text(GTK_LABEL(app->status_label), "Erro ao carregar arquivo OSM");
        }
        
        // Reset seleções
        app->has_start_point = FALSE;
        app->has_end_point = FALSE;
        app->selected_start_id = 0;
        app->selected_end_id = 0;
        
        gtk_widget_queue_draw(app->graph_area);
        printf("=============================\n\n");
        return TRUE;
    }
    
    return FALSE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    SimpleApp *app = g_new0(SimpleApp, 1);
    app->zoom_factor = 1.0;
    app->pan_x = 0.0;
    app->pan_y = 0.0;
    app->has_start_point = FALSE;
    app->has_end_point = FALSE;
    
    // Criar janela
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "Teste de Seleção de Pontos");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 800, 600);
    
    // Criar layout vertical
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(app->window), vbox);
    
    // Label de status
    app->status_label = gtk_label_new("Pressione 'O' para carregar arquivo OSM, depois Ctrl+clique para selecionar pontos");
    gtk_label_set_line_wrap(GTK_LABEL(app->status_label), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), app->status_label, FALSE, FALSE, 5);
    
    // Área de desenho
    app->graph_area = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), app->graph_area, TRUE, TRUE, 0);
    
    // Conectar sinais
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(app->graph_area, "draw", G_CALLBACK(on_draw_simple), app);
    g_signal_connect(app->graph_area, "button-press-event", G_CALLBACK(on_button_press_simple), app);
    g_signal_connect(app->window, "key-press-event", G_CALLBACK(on_key_press_simple), app);
    
    // Habilitar eventos
    gtk_widget_add_events(app->graph_area, GDK_BUTTON_PRESS_MASK);
    gtk_widget_set_can_focus(app->window, TRUE);
    
    gtk_widget_show_all(app->window);
    
    printf("=== TESTE DE SELEÇÃO INICIADO ===\n");
    printf("INSTRUÇÕES:\n");
    printf("1. Pressione 'O' para carregar arquivo OSM\n");
    printf("2. Use Ctrl+clique para selecionar pontos\n");
    printf("3. Primeiro clique = ponto inicial (verde)\n");
    printf("4. Segundo clique = ponto final (vermelho)\n");
    printf("5. Terceiro clique = reset e novo início\n");
    printf("=================================\n\n");
    
    gtk_main();
    
    if (app->grafo) {
        liberar_grafo(app->grafo);
    }
    g_free(app);
    
    return 0;
}
