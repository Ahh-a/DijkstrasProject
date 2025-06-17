#ifndef APP_DATA_H
#define APP_DATA_H

#include <gtk/gtk.h>
#include "graph.h"
#include "edit.h"

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
    
    // Variáveis para exibição do caminho mais curto
    long *shortest_path;
    int shortest_path_length;
    gboolean has_shortest_path;
    
    // Estado de edição
    EditState edit_state;
} AppData;

#endif // APP_DATA_H
