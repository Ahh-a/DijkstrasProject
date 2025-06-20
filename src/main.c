#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "app_data.h"
#include "ui_helpers.h"
#include "callbacks.h"
#include "drawing.h"
#include "interaction.h"

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
    
    // Inicializar dados básicos
    app->grafo = NULL;
    app->current_file = NULL;
    
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
    
    // Inicializar variáveis do caminho mais curto
    app->shortest_path = NULL;
    app->shortest_path_length = 0;
    app->has_shortest_path = FALSE;
    
    // Inicializar estado de edição
    app->edit_state.current_mode = EDIT_MODE_NONE;
    app->edit_state.is_connecting = FALSE;
    app->edit_state.connecting_from_id = 0;
    app->edit_state.has_selected_node = FALSE;
    app->edit_state.selected_node_id = 0;
    app->edit_state.next_node_id = 1;
    app->edit_state.next_way_id = 1;
    
    // Obter widgets com verificação de erro
    app->window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    if (!app->window) {
        g_error("Failed to get main_window from UI file");
        return 1;
    }
    
    app->file_label = GTK_WIDGET(gtk_builder_get_object(builder, "file_label"));
    if (!app->file_label) g_warning("Failed to get file_label from UI file");
    
    app->stats_label = GTK_WIDGET(gtk_builder_get_object(builder, "stats_label"));
    if (!app->stats_label) g_warning("Failed to get stats_label from UI file");
    
    app->start_entry = GTK_WIDGET(gtk_builder_get_object(builder, "start_entry"));
    if (!app->start_entry) g_warning("Failed to get start_entry from UI file");
    
    app->end_entry = GTK_WIDGET(gtk_builder_get_object(builder, "end_entry"));
    if (!app->end_entry) g_warning("Failed to get end_entry from UI file");
    
    app->results_text = GTK_WIDGET(gtk_builder_get_object(builder, "results_text"));
    if (!app->results_text) g_warning("Failed to get results_text from UI file");
    
    app->graph_area = GTK_WIDGET(gtk_builder_get_object(builder, "graph_area"));
    if (!app->graph_area) {
        g_error("Failed to get graph_area from UI file - this is critical!");
        return 1;
    }
    
    app->statusbar = GTK_WIDGET(gtk_builder_get_object(builder, "statusbar"));
    if (!app->statusbar) g_warning("Failed to get statusbar from UI file");
    
    // Configurar status bar
    if (app->statusbar) {
        app->status_context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(app->statusbar), "main");
    } else {
        app->status_context_id = 0;
    }
    
    // Conectar sinais com verificação de NULL
    if (app->window) {
        g_signal_connect(app->window, "delete-event", G_CALLBACK(on_window_delete), app);
    }
    
    GObject *widget;
    widget = gtk_builder_get_object(builder, "open_osm_item");
    if (widget) g_signal_connect(widget, "activate", G_CALLBACK(on_open_osm_clicked), app);
    
    widget = gtk_builder_get_object(builder, "quit_item");
    if (widget) g_signal_connect(widget, "activate", G_CALLBACK(on_quit_clicked), app);
    
    widget = gtk_builder_get_object(builder, "load_button");
    if (widget) g_signal_connect(widget, "clicked", G_CALLBACK(on_load_osm_clicked), app);
    
    widget = gtk_builder_get_object(builder, "clear_button");
    if (widget) g_signal_connect(widget, "clicked", G_CALLBACK(on_clear_clicked), app);
    
    widget = gtk_builder_get_object(builder, "export_png_button");
    if (widget) g_signal_connect(widget, "clicked", G_CALLBACK(on_export_png_clicked), app);
    
    widget = gtk_builder_get_object(builder, "find_path_button");
    if (widget) g_signal_connect(widget, "clicked", G_CALLBACK(on_find_path_clicked), app);
    
    widget = gtk_builder_get_object(builder, "edit_create_button");
    if (widget) g_signal_connect(widget, "clicked", G_CALLBACK(on_edit_create_clicked), app);
    
    widget = gtk_builder_get_object(builder, "edit_delete_button");
    if (widget) g_signal_connect(widget, "clicked", G_CALLBACK(on_edit_delete_clicked), app);
    
    widget = gtk_builder_get_object(builder, "edit_connect_button");
    if (widget) g_signal_connect(widget, "clicked", G_CALLBACK(on_edit_connect_clicked), app);
    
    widget = gtk_builder_get_object(builder, "edit_normal_button");
    if (widget) g_signal_connect(widget, "clicked", G_CALLBACK(on_edit_normal_clicked), app);
    
    if (app->graph_area) {
        g_signal_connect(app->graph_area, "draw", G_CALLBACK(on_graph_draw), app);
        g_signal_connect(app->graph_area, "button-press-event", G_CALLBACK(on_graph_button_press), app);
        g_signal_connect(app->graph_area, "button-release-event", G_CALLBACK(on_graph_button_release), app);
        g_signal_connect(app->graph_area, "motion-notify-event", G_CALLBACK(on_graph_motion_notify), app);
        g_signal_connect(app->graph_area, "scroll-event", G_CALLBACK(on_graph_scroll), app);
    }
    
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
    
    // Limpar memória
    if (app->shortest_path) {
        free(app->shortest_path);
    }
    if (app->grafo) {
        liberar_grafo(app->grafo);
    }
    g_free(app->current_file);
    g_free(app);
    g_object_unref(builder);
    
    return 0;
}