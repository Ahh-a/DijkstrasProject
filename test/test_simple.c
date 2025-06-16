/**
 * Simple test program to validate OSM file loading
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/osm_reader.h"

int main() {
    printf("Testing casaPrimo.osm file loading...\n");
    
    // Try to open the file first
    FILE* test_file = fopen("../casaPrimo.osm", "r");
    if (!test_file) {
        printf("ERROR: Cannot open casaPrimo.osm file\n");
        return 1;
    }
    printf("✓ File casaPrimo.osm can be opened\n");
    fclose(test_file);
    
    // Load the OSM file
    printf("Loading OSM file...\n");
    Grafo* graph = ler_osm("../casaPrimo.osm");
    if (!graph) {
        printf("ERROR: Failed to load casaPrimo.osm\n");
        return 1;
    }
    
    printf("✓ Successfully loaded casaPrimo.osm\n");
    printf("  - Total nodes: %zu\n", graph->num_pontos);
    printf("  - Total edges: %zu\n", graph->num_arestas);
    
    if (graph->num_arestas > 0) {
        printf("✓ Edges were created successfully\n");
        
        // Count bidirectional vs oneway
        int bidirectional = 0, oneway = 0;
        for (size_t i = 0; i < graph->num_arestas; i++) {
            if (graph->arestas[i].is_bidirectional) {
                bidirectional++;
            } else {
                oneway++;
            }
        }
        
        printf("  - Bidirectional edges: %d\n", bidirectional);
        printf("  - Oneway edges: %d\n", oneway);
    } else {
        printf("⚠ No edges were created\n");
    }
    
    // Clean up
    liberar_grafo(graph);
    printf("✓ Memory cleanup completed\n");
    
    return 0;
}
