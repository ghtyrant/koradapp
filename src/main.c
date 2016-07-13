#include <glib-unix.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <korad.h>
#include <string.h>
#include <unistd.h>

#include "koradapp.h"

gpointer
libkorad_run(gpointer user_data)
{
    AppState *state = (AppState*) user_data;
    korad_run(state->device);
    korad_device_free(state->device);

    return NULL;
}

GtkBuilder *
load_ui(AppState *state)
{
    GtkBuilder *builder;
    builder = gtk_builder_new();
    GError *error = NULL;
    gtk_builder_add_from_file(builder, "/home/lab/projects/koradapp/src/ui/koradview.glade", &error);

    if (error != NULL)
    {
        printf("Error loading user interface: %s", error->message);
        g_error_free(error);
        return NULL;
    }

    gtk_builder_connect_signals( builder, state );

    return builder;
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

    if (korad_device_find(&state->device) != KORAD_OK)
        return NULL;

    g_info("Found Korad Power Supply: %s", state->device->known_device->name);
    GThread *korad_thread = g_thread_new("KoradComm", libkorad_run, state);
    sleep(2);

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            gchar **config = g_strsplit(argv[i], ":", 2);
            if (strcmp(config[0], "main") == 0)
            {
                gulong socket_id = g_ascii_strtoull(config[1], NULL, 10);
                main_view_setup(socket_id, state);
            }
        }
    }
    else
    {
        GtkBuilder *builder = load_ui(state);
        GtkContainer *window = GTK_CONTAINER(gtk_builder_get_object(builder, "Main Window"));

        gtk_widget_show_all(GTK_WIDGET(window));
    }

    gtk_main();

    korad_device_stop(state->device);
    g_thread_join(korad_thread);

    g_info("Quitting Koradapp ...");

    return 0;
}
