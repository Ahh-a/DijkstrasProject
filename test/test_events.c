// Teste simples de eventos de mouse e teclado
#include <gtk/gtk.h>
#include <stdio.h>

gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    printf("=== BUTTON PRESS EVENT ===\n");
    printf("Button: %d\n", event->button);
    printf("Position: (%.2f, %.2f)\n", event->x, event->y);
    printf("State: %d\n", event->state);
    printf("GDK_CONTROL_MASK: %d\n", GDK_CONTROL_MASK);
    printf("Ctrl pressed: %s\n", (event->state & GDK_CONTROL_MASK) ? "YES" : "NO");
    printf("========================\n\n");
    
    return TRUE;
}

gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    // White background
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    // Instructions
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, 10, 30);
    cairo_show_text(cr, "Click normally and with Ctrl pressed to test event detection");
    cairo_move_to(cr, 10, 60);
    cairo_show_text(cr, "Check terminal output for debug information");
    
    return FALSE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Event Test");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), NULL);
    g_signal_connect(drawing_area, "button-press-event", G_CALLBACK(on_button_press), NULL);
    
    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK);
    
    gtk_widget_show_all(window);
    
    printf("Event test started. Click on the window to test.\n");
    
    gtk_main();
    
    return 0;
}
