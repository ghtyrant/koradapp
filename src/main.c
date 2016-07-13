#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <korad.h>

#include "plugin.h"
#include "koradapp.h"

void app_shutdown(gpointer user_data)
{
    gtk_main_quit();
}

gpointer
libkorad_run(gpointer user_data)
{
    AppState *state = (AppState*) user_data;

    if (korad_device_find(&state->device) != KORAD_OK)
        return NULL;

    g_info("Found Korad Power Supply: %s", state->device->known_device->name);

    korad_run(state->device);

    korad_device_free(state->device);
    return NULL;
}

void *on_embedded(gpointer user_data)
{
    AppState *state = (AppState*) user_data;

    g_thread_new("KoradComm", libkorad_run, state);

    return NULL;
}

void destroy_plugin()
{
    return;
}

GList*
initialize_plugin()
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

    AppState *state = g_new0(AppState, 1);

    g_signal_connect(plug, "embedded", G_CALLBACK(on_embedded), state);
    gtk_container_remove(window, box);
    gtk_container_add(GTK_CONTAINER(plug), box);

    gtk_widget_show_all(GTK_WIDGET(plug));

    GList *views = NULL;
    PluginViewDefinition *view = g_new0(PluginViewDefinition, 1);

    view->name = "Power Supply";
    view->icon = "";
    view->plug_id = gtk_plug_get_id(GTK_PLUG(plug));

    views = g_list_append(views, view);

    return views;

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
