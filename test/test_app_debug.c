#include <stdio.h>
#include <stdlib.h>
#include "../include/osm_reader.h"

int main() {
    printf("Testing OSM loading in application context...\n");
    
    // Simular o carregamento como na aplicação GTK
    Grafo* grafo = ler_osm("casaPrimo.osm");
    
    if (!grafo) {
        printf("Error: Failed to load OSM file\n");
        return 1;
    }
    
    printf("Successfully loaded:\n");
    printf("- Points: %zu\n", grafo->num_pontos);
    printf("- Edges: %zu\n", grafo->num_arestas);
    
    if (grafo->num_pontos > 0) {
        printf("\nFirst few points with full precision:\n");
        for (size_t i = 0; i < 5 && i < grafo->num_pontos; i++) {
            printf("  Point %zu: ID=%lld, lat=%.12f, lon=%.12f\n", 
                   i, grafo->pontos[i].id, 
                   grafo->pontos[i].lat, grafo->pontos[i].lon);
        }
        
        // Test bounds calculation like in GTK app
        double min_lat = 90.0, max_lat = -90.0;
        double min_lon = 180.0, max_lon = -180.0;
        
        for (size_t i = 0; i < grafo->num_pontos; i++) {
            if (grafo->pontos[i].lat < min_lat) min_lat = grafo->pontos[i].lat;
            if (grafo->pontos[i].lat > max_lat) max_lat = grafo->pontos[i].lat;
            if (grafo->pontos[i].lon < min_lon) min_lon = grafo->pontos[i].lon;
            if (grafo->pontos[i].lon > max_lon) max_lon = grafo->pontos[i].lon;
        }
        
        printf("\nCalculated bounds (should match previous test):\n");
        printf("  Latitude: %.12f to %.12f (range: %.12f)\n", min_lat, max_lat, max_lat - min_lat);
        printf("  Longitude: %.12f to %.12f (range: %.12f)\n", min_lon, max_lon, max_lon - min_lon);
        
        // Test scale calculation
        double lat_range = max_lat - min_lat;
        double lon_range = max_lon - min_lon;
        int widget_width = 500, widget_height = 560; // Typical values from GTK
        
        double scale_x = widget_width / lon_range;
        double scale_y = widget_height / lat_range;
        double scale = (scale_x < scale_y) ? scale_x : scale_y;
        scale *= 0.9; // Margin
        
        printf("\nScale calculation:\n");
        printf("  lat_range: %.12f\n", lat_range);
        printf("  lon_range: %.12f\n", lon_range);
        printf("  scale_x: %.12f\n", scale_x);
        printf("  scale_y: %.12f\n", scale_y);
        printf("  final_scale: %.12f\n", scale);
        
        // Test coordinate transformation for first point
        if (grafo->num_pontos > 0) {
            double x = (grafo->pontos[0].lon - min_lon) * scale + (widget_width - lon_range * scale) / 2;
            double y = widget_height - ((grafo->pontos[0].lat - min_lat) * scale + (widget_height - lat_range * scale) / 2);
            printf("\nFirst point screen coordinates:\n");
            printf("  x: %.2f, y: %.2f\n", x, y);
        }
    }
    
    liberar_grafo(grafo);
    printf("\nTest completed successfully!\n");
    return 0;
}
