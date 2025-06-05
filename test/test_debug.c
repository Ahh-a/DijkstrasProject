#include <stdio.h>
#include <stdlib.h>
#include "../include/osm_reader.h"

int main() {
    printf("Testing casaPrimo.osm loading...\n");
    
    Grafo* grafo = ler_osm("casaPrimo.osm");
    
    if (!grafo) {
        printf("Error: Failed to load OSM file\n");
        return 1;
    }
    
    printf("Successfully loaded:\n");
    printf("- Points: %zu\n", grafo->num_pontos);
    printf("- Edges: %zu\n", grafo->num_arestas);
    
    if (grafo->num_pontos > 0) {
        printf("\nFirst few points:\n");
        for (size_t i = 0; i < 5 && i < grafo->num_pontos; i++) {
            printf("  Point %zu: ID=%lld, lat=%.6f, lon=%.6f\n", 
                   i, grafo->pontos[i].id, 
                   grafo->pontos[i].lat, grafo->pontos[i].lon);
        }
        
        // Calculate bounds
        double min_lat = 90.0, max_lat = -90.0;
        double min_lon = 180.0, max_lon = -180.0;
        
        for (size_t i = 0; i < grafo->num_pontos; i++) {
            if (grafo->pontos[i].lat < min_lat) min_lat = grafo->pontos[i].lat;
            if (grafo->pontos[i].lat > max_lat) max_lat = grafo->pontos[i].lat;
            if (grafo->pontos[i].lon < min_lon) min_lon = grafo->pontos[i].lon;
            if (grafo->pontos[i].lon > max_lon) max_lon = grafo->pontos[i].lon;
        }
        
        printf("\nBounds:\n");
        printf("  Latitude: %.6f to %.6f (range: %.6f)\n", min_lat, max_lat, max_lat - min_lat);
        printf("  Longitude: %.6f to %.6f (range: %.6f)\n", min_lon, max_lon, max_lon - min_lon);
    }
    
    if (grafo->num_arestas > 0) {
        printf("\nFirst few edges:\n");
        for (size_t i = 0; i < 5 && i < grafo->num_arestas; i++) {
            printf("  Edge %zu: %lld -> %lld (weight: %.2f)\n", 
                   i, grafo->arestas[i].origem, 
                   grafo->arestas[i].destino, grafo->arestas[i].peso);
        }
    }
    
    liberar_grafo(grafo);
    printf("\nTest completed successfully!\n");
    return 0;
}
