#include "../include/osm_reader.h"
#include <stdio.h>

int main() {
    printf("Testing actual OSM file loading...\n");
    
    Grafo* grafo = ler_osm("casaPrimo.osm");
    if (!grafo) {
        printf("Failed to load OSM file\n");
        return 1;
    }
    
    printf("Loaded %zu points and %zu edges\n", grafo->num_pontos, grafo->num_arestas);
    
    // Print first few points with careful formatting
    printf("\nFirst 5 points:\n");
    for (int i = 0; i < 5 && i < grafo->num_pontos; i++) {
        Ponto *p = &grafo->pontos[i];
        printf("Point %d: ID=%lld, lat=%.9f, lon=%.9f\n", i, p->id, p->lat, p->lon);
        printf("         lat hex=%a, lon hex=%a\n", p->lat, p->lon);
    }
    
    // Calculate bounds
    double min_lat = 90.0, max_lat = -90.0;
    double min_lon = 180.0, max_lon = -180.0;
    
    for (size_t i = 0; i < grafo->num_pontos; i++) {
        Ponto *p = &grafo->pontos[i];
        if (p->lat < min_lat) min_lat = p->lat;
        if (p->lat > max_lat) max_lat = p->lat;
        if (p->lon < min_lon) min_lon = p->lon;
        if (p->lon > max_lon) max_lon = p->lon;
    }
    
    printf("\nBounds:\n");
    printf("Latitude: %.9f to %.9f (range: %.9f)\n", min_lat, max_lat, max_lat - min_lat);
    printf("Longitude: %.9f to %.9f (range: %.9f)\n", min_lon, max_lon, max_lon - min_lon);
    
    liberar_grafo(grafo);
    return 0;
}
