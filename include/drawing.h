#ifndef DRAWING_H
#define DRAWING_H

#include "app_data.h"

// Função para desenhar o grafo
gboolean on_graph_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);

// Função para renderizar o grafo em uma superfície off-screen e salvar como PNG
void render_graph_to_png(AppData *app, const char *filename, int width, int height);

#endif // DRAWING_H
