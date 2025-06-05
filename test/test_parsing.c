#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

int main() {
    printf("Testing OSM coordinate parsing...\n");
    
    // Simulate the exact strings from OSM file
    char lat_str[] = "-16.6988864";
    char lon_str[] = "-49.1914952";
    
    printf("Before parsing:\n");
    printf("lat_str='%s', lon_str='%s'\n", lat_str, lon_str);
    
    // Test with current locale
    double lat = strtod(lat_str, NULL);
    double lon = strtod(lon_str, NULL);
    
    printf("After strtod():\n");
    printf("lat = %f, lon = %f\n", lat, lon);
    printf("lat = %.6f, lon = %.6f\n", lat, lon);
    
    // Test printf formatting
    printf("Formatted output:\n");
    printf("lat: %f, lon: %f\n", lat, lon);
    
    return 0;
}
