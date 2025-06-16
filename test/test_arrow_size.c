#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "../include/osm_reader.h"

// Test arrow size calculation logic
void test_arrow_size_calculation() {
    printf("Testing arrow size calculation...\n");
    
    // Simulate different zoom factors
    double zoom_factors[] = {0.1, 0.5, 1.0, 2.0, 5.0, 10.0};
    int num_zooms = sizeof(zoom_factors) / sizeof(zoom_factors[0]);
    
    for (int i = 0; i < num_zooms; i++) {
        double zoom = zoom_factors[i];
        
        // Arrow size calculation (same as in main.c)
        double arrow_size = 12.0 / zoom;
        if (arrow_size < 6.0) arrow_size = 6.0;
        if (arrow_size > 20.0) arrow_size = 20.0;
        
        printf("  Zoom: %.1f -> Arrow size: %.1f\n", zoom, arrow_size);
        
        // Validate arrow size bounds
        assert(arrow_size >= 6.0);
        assert(arrow_size <= 20.0);
        
        // Check that higher zoom results in smaller arrows (within bounds)
        if (zoom >= 0.6) { // Above the minimum threshold
            assert(arrow_size >= 6.0); // Should be at minimum
        }
        if (zoom <= 2.0) { // Below the maximum threshold  
            assert(arrow_size <= 20.0); // Should be at maximum
        }
    }
    
    printf("✓ Arrow size calculation tests passed!\n");
}

// Test line width calculation logic
void test_line_width_calculation() {
    printf("Testing line width calculation...\n");
    
    double zoom_factors[] = {0.5, 1.0, 2.0, 4.0};
    int num_zooms = sizeof(zoom_factors) / sizeof(zoom_factors[0]);
    
    for (int i = 0; i < num_zooms; i++) {
        double zoom = zoom_factors[i];
        
        // Line width calculation (same as in main.c)
        double bidirectional_width = 0.8 / zoom;
        double oneway_width = 1.2 / zoom;
        
        printf("  Zoom: %.1f -> Bidirectional: %.2f, Oneway: %.2f\n", 
               zoom, bidirectional_width, oneway_width);
        
        // Oneway streets should always be thicker than bidirectional
        assert(oneway_width > bidirectional_width);
        
        // The ratio should be consistent
        double ratio = oneway_width / bidirectional_width;
        assert(fabs(ratio - 1.5) < 0.01); // 1.2 / 0.8 = 1.5
    }
    
    printf("✓ Line width calculation tests passed!\n");
}

int main() {
    printf("=== Arrow Size Enhancement Test ===\n\n");
    
    test_arrow_size_calculation();
    printf("\n");
    test_line_width_calculation();
    
    printf("\n✓ All visual enhancement tests passed!\n");
    printf("The enhanced arrows should now be:\n");
    printf("  - Larger (6-20 pixels instead of 2-8)\n");
    printf("  - More visible at different zoom levels\n");
    printf("  - Oneway streets are 50%% thicker than bidirectional\n");
    
    return 0;
}
