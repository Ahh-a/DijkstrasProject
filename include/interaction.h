#ifndef INTERACTION_H
#define INTERACTION_H

#include "app_data.h"

// Callback para zoom com scroll do mouse
gboolean on_graph_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);

// Função para encontrar o ponto mais próximo do clique
Ponto* find_closest_point(AppData *app, double click_x, double click_y);

// Função para converter coordenadas de tela para lat/lon
void screen_to_latlon(AppData *app, double screen_x, double screen_y, double *lat, double *lon);

// Função para lidar com cliques no modo de edição
gboolean handle_edit_click(AppData *app, double click_x, double click_y, GtkWidget *widget);

// Callback para cliques no botão do mouse na área do grafo
gboolean on_graph_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

// Callback para fim do drag (botão liberado)
gboolean on_graph_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

// Callback para movimento do mouse (drag)
gboolean on_graph_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);

#endif // INTERACTION_H
