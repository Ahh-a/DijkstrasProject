#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "app_data.h"

// Callback para abrir arquivo OSM
void on_open_osm_clicked(GtkMenuItem *menuitem, gpointer user_data);

// Callback para load OSM via toolbar
void on_load_osm_clicked(GtkToolButton *toolbutton, gpointer user_data);

// Callback para encontrar caminho mais curto
void on_find_path_clicked(GtkButton *button, gpointer user_data);

// Callback para limpar
void on_clear_clicked(GtkToolButton *toolbutton, gpointer user_data);

// Callback para sair
void on_quit_clicked(GtkMenuItem *menuitem, gpointer user_data);

// Callback para fechamento da janela
gboolean on_window_delete(GtkWidget *widget, GdkEvent *event, gpointer user_data);

// Callback functions for edit mode buttons
void on_edit_create_clicked(GtkToolButton *toolbutton, gpointer user_data);
void on_edit_delete_clicked(GtkToolButton *toolbutton, gpointer user_data);
void on_edit_connect_clicked(GtkToolButton *toolbutton, gpointer user_data);
void on_edit_normal_clicked(GtkToolButton *toolbutton, gpointer user_data);

// Callback para exportar PNG
void on_export_png_clicked(GtkToolButton *toolbutton, gpointer user_data);

#endif // CALLBACKS_H
