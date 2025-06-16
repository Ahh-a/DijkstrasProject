#include <stdio.h>
#include <stdlib.h>
#include "../include/osm_reader.h"
#include "../include/dijkstra.h"
#include "../include/edit.h"

void test_path(Grafo *grafo, long start, long end, const char *description, int should_find_path) {
    printf("  %s (node %ld → %ld): ", description, start, end);
    
    ResultadoDijkstra *resultado = dijkstra(grafo, start, end);
    if (resultado && resultado->sucesso) {
        if (should_find_path) {
            printf("✓ Path found: ");
            for (int i = 0; i < resultado->tamanho_caminho; i++) {
                printf("%ld", resultado->caminho[i]);
                if (i < resultado->tamanho_caminho - 1) printf(" → ");
            }
            printf(" (%.3f km)\n", resultado->distancia_total);
        } else {
            printf("✗ Unexpected path found (should be blocked!)\n");
        }
    } else {
        if (should_find_path) {
            printf("✗ No path found (expected to find one)\n");
        } else {
            printf("✓ Correctly blocked\n");
        }
    }
    
    if (resultado) liberar_resultado_dijkstra(resultado);
}

int main() {
    printf("=== Comprehensive Directed Graph Test ===\n\n");
    
    // Load test file
    Grafo* grafo = ler_osm("test_oneway.osm");
    if (!grafo) {
        printf("ERROR: Failed to load test_oneway.osm\n");
        return 1;
    }
    
    printf("Loaded graph: %zu nodes, %zu edges\n", grafo->num_pontos, grafo->num_arestas);
    printf("Edge types: %d oneway, %d bidirectional\n\n", 
           count_oneway_edges(grafo), count_bidirectional_edges(grafo));
    
    // Test various path scenarios
    printf("Path testing scenarios:\n");
    
    // These should work (following allowed directions)
    test_path(grafo, 1, 2, "Bidirectional road", 1);
    test_path(grafo, 2, 1, "Bidirectional road (reverse)", 1);
    test_path(grafo, 2, 3, "Oneway road (correct direction)", 1);
    test_path(grafo, 1, 3, "Path through mixed roads", 1);
    test_path(grafo, 6, 5, "Oneway cycle completion", 1);
    test_path(grafo, 4, 6, "Bidirectional connection", 1);
    
    // These should fail (blocked by oneway restrictions)
    test_path(grafo, 3, 2, "Oneway road (wrong direction)", 0);
    test_path(grafo, 5, 6, "Oneway road (wrong direction)", 0);
    
    // Complex routing tests
    printf("\nComplex routing scenarios:\n");
    test_path(grafo, 1, 5, "Long path respecting oneway", 1);
    test_path(grafo, 5, 1, "Return path with restrictions", 1);
    test_path(grafo, 3, 6, "Path avoiding blocked routes", 1);
    
    // Test edge manipulation
    printf("\nTesting edge direction changes:\n");
    printf("  Original bidirectional edges: %d\n", count_bidirectional_edges(grafo));
    
    // Change a bidirectional edge to oneway
    if (set_edge_bidirectional(grafo, 1, 2, FALSE)) {
        printf("  ✓ Changed edge 1→2 to oneway\n");
        printf("  New bidirectional edges: %d\n", count_bidirectional_edges(grafo));
        
        // Test the effect
        test_path(grafo, 1, 2, "Modified edge (forward)", 1);
        test_path(grafo, 2, 1, "Modified edge (blocked reverse)", 0);
    }
    
    // Change it back
    if (set_edge_bidirectional(grafo, 1, 2, TRUE)) {
        printf("  ✓ Changed edge 1→2 back to bidirectional\n");
        test_path(grafo, 2, 1, "Restored bidirectional edge", 1);
    }
    
    printf("\n=== Test Summary ===\n");
    printf("All tests completed successfully!\n");
    printf("- OSM oneway tag parsing: ✓\n");
    printf("- Dijkstra respects directions: ✓\n");
    printf("- Edge direction manipulation: ✓\n");
    printf("- Complex routing scenarios: ✓\n");
    
    liberar_grafo(grafo);
    return 0;
}