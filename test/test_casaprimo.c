/**
 * Test program to validate directed graph functionality with casaPrimo.osm
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/osm_reader.h"

int main() {
    printf("Testing casaPrimo.osm directed graph loading...\n");
    
    // Load the OSM file
    Grafo* graph = ler_osm("../casaPrimo.osm");
    if (!graph) {
        printf("ERROR: Failed to load casaPrimo.osm\n");
        return 1;
    }
    
    printf("✓ Successfully loaded casaPrimo.osm\n");
    printf("  - Total nodes: %zu\n", graph->num_pontos);
    printf("  - Total edges: %zu\n", graph->num_arestas);
    
    // Count directed vs bidirectional edges
    int oneway_edges = 0;
    int bidirectional_edges = 0;
    
    for (size_t i = 0; i < graph->num_arestas; i++) {
        if (graph->arestas[i].is_bidirectional) {
            bidirectional_edges++;
        } else {
            oneway_edges++;
        }
    }
    
    printf("  - Oneway edges: %d\n", oneway_edges);
    printf("  - Bidirectional edges: %d\n", bidirectional_edges);
    
    // Verify our directed graph logic
    if (oneway_edges > 0) {
        printf("✓ Successfully detected oneway streets from OSM data\n");
    } else {
        printf("⚠ No oneway streets detected - check parsing logic\n");
    }
    
    if (bidirectional_edges > 0) {
        printf("✓ Successfully detected bidirectional streets\n");
    } else {
        printf("⚠ No bidirectional streets detected\n");
    }
    
    // Show some example edges
    printf("\nFirst 5 edges:\n");
    for (size_t i = 0; i < graph->num_arestas && i < 5; i++) {
        printf("  Edge %zu: %lld -> %lld (weight: %.2f, %s)\n", 
               i, 
               graph->arestas[i].origem,
               graph->arestas[i].destino,
               graph->arestas[i].peso,
               graph->arestas[i].is_bidirectional ? "bidirectional" : "oneway");
    }
    
    // Clean up
    liberar_grafo(graph);
    printf("\n✓ Memory cleanup completed\n");
    
    printf("\nTest completed successfully!\n");
    return 0;
}
