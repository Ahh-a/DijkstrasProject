#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

int main() {
    printf("System locale: %s\n", setlocale(LC_NUMERIC, NULL));
    
    // Test parsing with system locale
    char test_str[] = "-16.6988864";
    double val1 = strtod(test_str, NULL);
    printf("With system locale: strtod('%s') = %f\n", test_str, val1);
    
    // Set C locale
    setlocale(LC_NUMERIC, "C");
    printf("After setting C locale: %s\n", setlocale(LC_NUMERIC, NULL));
    
    double val2 = strtod(test_str, NULL);
    printf("With C locale: strtod('%s') = %f\n", test_str, val2);
    
    return 0;
}
