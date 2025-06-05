#include "../include/osm_reader.h"
#include <stdio.h>

int main() {
    printf("Testing coordinate transformation for drawing...\n");
    
    Grafo* grafo = ler_osm("casaPrimo.osm");
    if (!grafo) {
        printf("Failed to load OSM file\n");
        return 1;
    }
    
    printf("Loaded %zu points\n", grafo->num_pontos);
    
    // Calculate bounds like in the drawing function
    double min_lat = 90.0, max_lat = -90.0;
    double min_lon = 180.0, max_lon = -180.0;
    
    for (size_t i = 0; i < grafo->num_pontos; i++) {
        Ponto *p = &grafo->pontos[i];
        if (p->lat < min_lat) min_lat = p->lat;
        if (p->lat > max_lat) max_lat = p->lat;
        if (p->lon < min_lon) min_lon = p->lon;
        if (p->lon > max_lon) max_lon = p->lon;
    }
    
    double lat_range = max_lat - min_lat;
    double lon_range = max_lon - min_lon;
    
    printf("Bounds: lat [%.6f, %.6f] range %.6f\n", min_lat, max_lat, lat_range);
    printf("        lon [%.6f, %.6f] range %.6f\n", min_lon, max_lon, lon_range);
    
    // Simulate widget size
    int width = 500, height = 400;
    double scale_x = width / lon_range;
    double scale_y = height / lat_range;
    double scale = (scale_x < scale_y) ? scale_x : scale_y;
    scale *= 0.9;
    
    printf("Scale factors: x=%.2f, y=%.2f, final=%.2f\n", scale_x, scale_y, scale);
    
    // Transform first 5 points
    printf("\nFirst 5 points transformation:\n");
    for (int i = 0; i < 5 && i < grafo->num_pontos; i++) {
        Ponto *p = &grafo->pontos[i];
        double x = (p->lon - min_lon) * scale;
        double y = (max_lat - p->lat) * scale;
        
        printf("Point %d: (%.6f,%.6f) -> screen (%.1f,%.1f)\n", 
               i, p->lat, p->lon, x, y);
    }
    
    liberar_grafo(grafo);
    return 0;
}
