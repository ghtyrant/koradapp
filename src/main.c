#include <glib-unix.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <korad.h>
#include <string.h>

#include "koradapp.h"

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

GtkBuilder *
load_ui()
{
    GtkBuilder *builder;
    builder = gtk_builder_new();
    GError *error = NULL;
    gtk_builder_add_from_file(builder, "../koradapp/src/ui/koradview.glade", &error);

    if (error != NULL)
    {
        printf("Error loading user interface: %s", error->message);
        g_error_free(error);
        return NULL;
    }

    return builder;
}

void
on_output_toggle(GtkToggleButton *button, gpointer user_data)
{
    AppState *state = (AppState*) user_data;

    korad_output(state->device, gtk_toggle_button_get_active(button));
}

void
setup_view_main(gulong socket_id, AppState *state)
{
    GtkBuilder *builder = load_ui();
    GtkContainer *window = GTK_CONTAINER(gtk_builder_get_object(builder, "Main Window"));
    GtkWidget *box = GTK_WIDGET(gtk_builder_get_object(builder, "Outer Box"));
    g_object_ref(box);
    g_info("Starting view 'main' on socket %lu", socket_id);
    GtkWidget *plug = gtk_plug_new(socket_id);


    gtk_container_remove(window, box);
    gtk_container_add(GTK_CONTAINER(plug), box);
    gtk_widget_show_all(GTK_WIDGET(plug));

    GtkWidget *output = GTK_WIDGET(gtk_builder_get_object(builder, "Output"));
    g_signal_connect(output, "toggled", G_CALLBACK(on_output_toggle), state);

}

gboolean
handle_shutdown(gpointer user_data)
{
    gtk_main_quit();
    return FALSE;
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    g_unix_signal_add(SIGTERM, G_CALLBACK(handle_shutdown), NULL);

    AppState *state = g_new0(AppState, 1);

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            gchar **config = g_strsplit(argv[i], ":", 2);
            if (strcmp(config[0], "main") == 0)
            {
                gulong socket_id = g_ascii_strtoull(config[1], NULL, 10);
                setup_view_main(socket_id, state);
            }
        }
    }
    else
    {
        GtkBuilder *builder = load_ui();
        GtkContainer *window = GTK_CONTAINER(gtk_builder_get_object(builder, "Main Window"));

        gtk_widget_show_all(GTK_WIDGET(window));

        GtkWidget *output = GTK_WIDGET(gtk_builder_get_object(builder, "Output"));
        g_signal_connect(output, "toggled", G_CALLBACK(on_output_toggle), state);
    }

    GThread *korad_thread = g_thread_new("KoradComm", libkorad_run, state);

    gtk_main();

    korad_device_stop(state->device);
    g_thread_join(korad_thread);

    g_info("Quitting Koradapp ...");

    return 0;
}
