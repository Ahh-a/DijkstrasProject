#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "../include/osm_reader.h"
#include "../include/graph.h"
#include "../src/main.c"

// Teste da funcionalidade de exportar PNG
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    printf("Testing PNG export functionality...\n");
    
    // Carregar arquivo OSM de teste
    Grafo *grafo = ler_osm("../casaPrimo.osm");
    if (!grafo) {
        printf("ERROR: Failed to load OSM file\n");
        return 1;
    }
    
    printf("Loaded graph with %zu nodes and %zu edges\n", 
           grafo->num_pontos, grafo->num_arestas);
    
    // Criar AppData simulado
    AppData app_data = {0};
    app_data.grafo = grafo;
    app_data.zoom_factor = 1.0;
    app_data.pan_x = 0.0;
    app_data.pan_y = 0.0;
    app_data.has_start_point = FALSE;
    app_data.has_end_point = FALSE;
    app_data.has_shortest_path = FALSE;
    
    // Testar exportação PNG
    printf("Exporting graph to PNG...\n");
    render_graph_to_png(&app_data, "test_export.png", 1920, 1080);
    
    printf("PNG exported successfully!\n");
    printf("Check the file 'test_export.png' in the current directory.\n");
    
    // Limpar
    liberar_grafo(grafo);
    
    return 0;
}
