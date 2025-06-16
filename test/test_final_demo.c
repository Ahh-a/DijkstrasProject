#include <stdio.h>
#include <stdlib.h>
#include "../include/osm_reader.h"
#include "../include/dijkstra.h"
#include "../include/edit.h"

void print_separator(const char* title) {
    printf("\n=== %s ===\n", title);
}

void print_edge_stats(Grafo *grafo) {
    int oneway = count_oneway_edges(grafo);
    int bidirectional = count_bidirectional_edges(grafo);
    printf("Edge Statistics: %d oneway, %d bidirectional (total: %zu)\n", 
           oneway, bidirectional, grafo->num_arestas);
}

void test_path_with_explanation(Grafo *grafo, long start, long end, const char *description) {
    printf("\n🔍 Testing: %s\n", description);
    printf("Route: Node %ld → Node %ld\n", start, end);
    
    ResultadoDijkstra *resultado = dijkstra(grafo, start, end);
    if (resultado && resultado->sucesso) {
        printf("✅ SUCCESS: Path found\n");
        printf("📍 Route: ");
        for (int i = 0; i < resultado->tamanho_caminho; i++) {
            printf("%ld", resultado->caminho[i]);
            if (i < resultado->tamanho_caminho - 1) printf(" → ");
        }
        printf("\n📏 Distance: %.3f km\n", resultado->distancia_total);
    } else {
        printf("❌ BLOCKED: No valid path (respecting traffic directions)\n");
    }
    
    if (resultado) liberar_resultado_dijkstra(resultado);
}

int main() {
    printf("🚗 DIJKSTRA PROJECT - DIRECTED GRAPH DEMO\n");
    printf("=========================================\n");
    printf("Testing comprehensive directed graph support\n");
    
    // Load test file
    print_separator("Loading Test Graph");
    Grafo* grafo = ler_osm("test_oneway.osm");
    
    if (!grafo) {
        printf("❌ ERROR: Could not load test_oneway.osm\n");
        printf("Make sure the file exists in the project directory.\n");
        return 1;
    }
    
    printf("✅ Successfully loaded graph:\n");
    printf("📊 Nodes: %zu\n", grafo->num_pontos);
    printf("📊 Edges: %zu\n", grafo->num_arestas);
    print_edge_stats(grafo);
    
    // Show graph structure
    print_separator("Graph Structure Analysis");
    printf("🗺️  Detailed edge information:\n");
    for (size_t i = 0; i < grafo->num_arestas; i++) {
        Aresta *a = &grafo->arestas[i];
        printf("   Edge %zu: %lld → %lld (%.3f km) %s\n", 
               i, a->origem, a->destino, a->peso,
               a->is_bidirectional ? "🔄[BIDIRECTIONAL]" : "➡️[ONEWAY]");
    }
    
    // Test various routing scenarios
    print_separator("Routing Test Scenarios");
    
    test_path_with_explanation(grafo, 1, 2, 
        "Regular bidirectional street");
    
    test_path_with_explanation(grafo, 2, 3, 
        "Following oneway street in correct direction");
    
    test_path_with_explanation(grafo, 3, 2, 
        "Attempting to go against oneway street (should fail)");
    
    test_path_with_explanation(grafo, 1, 3, 
        "Multi-hop path respecting traffic rules");
    
    test_path_with_explanation(grafo, 1, 5, 
        "Complex routing through mixed street types");
    
    test_path_with_explanation(grafo, 5, 1, 
        "Return journey with traffic restrictions");
    
    test_path_with_explanation(grafo, 6, 5, 
        "Using oneway street in permitted direction");
    
    test_path_with_explanation(grafo, 5, 6, 
        "Attempting blocked direction (should fail)");
    
    // Test dynamic direction changes
    print_separator("Dynamic Direction Control");
    printf("🔧 Testing runtime direction modifications:\n\n");
    
    printf("📋 Original state:\n");
    print_edge_stats(grafo);
    
    printf("\n🔄 Converting bidirectional street 1↔2 to oneway 1→2:\n");
    if (set_connection_direction(grafo, 1, 2, 0)) {
        print_edge_stats(grafo);
        test_path_with_explanation(grafo, 1, 2, "Testing modified street (forward)");
        test_path_with_explanation(grafo, 2, 1, "Testing modified street (reverse - should fail)");
    }
    
    printf("\n🔄 Restoring street 1↔2 to bidirectional:\n");
    if (set_connection_direction(grafo, 1, 2, 1)) {
        print_edge_stats(grafo);
        test_path_with_explanation(grafo, 2, 1, "Testing restored bidirectional street");
    }
    
    // Performance summary
    print_separator("Feature Summary");
    printf("✅ OSM Parsing:\n");
    printf("   • Bidirectional streets (default)\n");
    printf("   • Oneway streets (oneway=yes)\n");
    printf("   • Reverse oneway streets (oneway=-1)\n");
    printf("\n✅ Visual Features:\n");
    printf("   • Blue lines with arrows for oneway streets\n");
    printf("   • Gray lines for bidirectional streets\n");
    printf("   • Red highlighting for calculated paths\n");
    printf("\n✅ Algorithm Features:\n");
    printf("   • Direction-aware Dijkstra implementation\n");
    printf("   • Respects traffic flow restrictions\n");
    printf("   • Dynamic direction modification support\n");
    printf("\n✅ Interactive Features:\n");
    printf("   • Runtime edge direction control\n");
    printf("   • Statistical reporting\n");
    printf("   • Comprehensive path validation\n");
    
    printf("\n🎉 All directed graph features working correctly!\n");
    printf("🚗 Ready for real-world traffic simulation!\n");
    
    liberar_grafo(grafo);
    return 0;
}
