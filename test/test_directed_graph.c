#include <stdio.h>
#include <stdlib.h>
#include "../include/osm_reader.h"
#include "../include/dijkstra.h"
#include "../include/edit.h"

int main() {
    printf("=== Testing Directed Graph Support ===\n\n");
    
    // Test loading OSM file with oneway tags
    printf("1. Loading test_oneway.osm...\n");
    Grafo* grafo = ler_osm("test_oneway.osm");
    
    if (!grafo) {
        printf("ERROR: Failed to load test_oneway.osm\n");
        return 1;
    }
    
    printf("✓ Successfully loaded:\n");
    printf("  - Nodes: %zu\n", grafo->num_pontos);
    printf("  - Edges: %zu\n", grafo->num_arestas);
    
    // Count edge types
    int oneway_count = count_oneway_edges(grafo);
    int bidirectional_count = count_bidirectional_edges(grafo);
    
    printf("  - Oneway edges: %d\n", oneway_count);
    printf("  - Bidirectional edges: %d\n", bidirectional_count);
    
    // Display all edges with their types
    printf("\n2. Edge details:\n");
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        Aresta *a = &grafo->arestas[i];
        printf("  Edge %zu: %lld → %lld (%.3f km) %s\n", 
               i, a->origem, a->destino, a->peso,
               a->is_bidirectional ? "[BIDIRECTIONAL]" : "[ONEWAY]");
    }
    
    // Test Dijkstra with directed edges
    printf("\n3. Testing Dijkstra with directed graph:\n");
    
    // Test case 1: Should find path respecting directions
    printf("  Test 3.1: Path from node 1 to node 3\n");
    ResultadoDijkstra *resultado1 = dijkstra(grafo, 1, 3);
    if (resultado1 && resultado1->sucesso) {
        printf("    ✓ Path found: ");
        for (int i = 0; i < resultado1->tamanho_caminho; i++) {
            printf("%ld", resultado1->caminho[i]);
            if (i < resultado1->tamanho_caminho - 1) printf(" → ");
        }
        printf(" (distance: %.3f km)\n", resultado1->distancia_total);
    } else {
        printf("    ✗ No path found\n");
    }
    
    // Test case 2: Should respect oneway restrictions
    printf("  Test 3.2: Path from node 3 to node 2 (should fail due to oneway)\n");
    ResultadoDijkstra *resultado2 = dijkstra(grafo, 3, 2);
    if (resultado2 && resultado2->sucesso) {
        printf("    ✗ Unexpected path found: ");
        for (int i = 0; i < resultado2->tamanho_caminho; i++) {
            printf("%ld", resultado2->caminho[i]);
            if (i < resultado2->tamanho_caminho - 1) printf(" → ");
        }
        printf(" (This should not happen with oneway restrictions!)\n");
    } else {
        printf("    ✓ Correctly blocked by oneway restriction\n");
    }
    
    // Test case 3: Test reverse oneway
    printf("  Test 3.3: Path from node 4 to node 5 (reverse oneway)\n");
    ResultadoDijkstra *resultado3 = dijkstra(grafo, 4, 5);
    if (resultado3 && resultado3->sucesso) {
        printf("    ✓ Path found using reverse oneway: ");
        for (int i = 0; i < resultado3->tamanho_caminho; i++) {
            printf("%ld", resultado3->caminho[i]);
            if (i < resultado3->tamanho_caminho - 1) printf(" → ");
        }
        printf(" (distance: %.3f km)\n", resultado3->distancia_total);
    } else {
        printf("    ✗ No path found\n");
    }
    
    // Test edge direction functions
    printf("\n4. Testing edge direction functions:\n");
    printf("  Is edge 1→2 bidirectional? %s\n", is_edge_bidirectional(grafo, 1, 2) ? "YES" : "NO");
    printf("  Is edge 2→3 bidirectional? %s\n", is_edge_bidirectional(grafo, 2, 3) ? "YES" : "NO");
    
    // Test setting edge direction
    printf("  Setting edge 1→2 to oneway...\n");
    if (set_edge_bidirectional(grafo, 1, 2, FALSE)) {
        printf("  ✓ Successfully changed edge direction\n");
        printf("  New oneway count: %d\n", count_oneway_edges(grafo));
        printf("  New bidirectional count: %d\n", count_bidirectional_edges(grafo));
    } else {
        printf("  ✗ Failed to change edge direction\n");
    }
    
    // Cleanup
    if (resultado1) liberar_resultado_dijkstra(resultado1);
    if (resultado2) liberar_resultado_dijkstra(resultado2);
    if (resultado3) liberar_resultado_dijkstra(resultado3);
    liberar_grafo(grafo);
    
    printf("\n=== Test completed ===\n");
    return 0;
}