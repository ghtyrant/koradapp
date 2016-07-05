#include <gtk/gtk.h>
#include <gtk/gtkx.h>

void app_shutdown(gpointer user_data)
{
    gtk_main_quit();
}

void on_embedded()
{
    printf("Embedded!\n");
}

gulong[] initialize_plugin()
{
    GtkBuilder *builder;
    builder = gtk_builder_new();
    GError *error = NULL;
    gtk_builder_add_from_file(builder, "../koradapp/src/ui/koradview.glade", &error);

    if (error != NULL)
    {
        printf("Error loading user interface: %s", error->message);
        g_error_free(error);
        return 0;
    }

    GtkContainer *window = GTK_CONTAINER(gtk_builder_get_object(builder, "Main Window"));
    GtkWidget *box = GTK_WIDGET(gtk_builder_get_object(builder, "Outer Box"));

    g_object_ref(box);

    GtkWidget *plug = gtk_plug_new(0L);

    g_signal_connect(plug, "embedded", on_embedded, NULL);
    gtk_container_remove(window, box);
    gtk_container_add(GTK_CONTAINER(plug), box);

    gtk_widget_show_all(GTK_WIDGET(plug));

    printf("Plug-ID: %lu\n", gtk_plug_get_id(GTK_PLUG(plug)));
    return { gtk_plug_get_id(GTK_PLUG(plug)), 0 };


    /* Set up CSS style provider */
    /*GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen,
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(provider),
        "src/ui/style.css", &error);

    if (error != NULL)
    {
        printf("Error loading user interface style: %s", error->message);
        g_error_free(error);
        return 0;
    }

    g_object_unref(provider);
    */

    return 0;
}
