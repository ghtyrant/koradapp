#ifndef KORADAPP_H_
#define KORADAPP_H_

typedef struct
{
    KoradDevice *device;
    guchar voltage[5];
    guchar current[5];

    // Main View
    GtkBuilder *main_builder;
    GtkEntry *voltage_entry[4];
    GtkEntry *current_entry[4];
    gboolean save;
    GtkToggleButton *save_toggle;
} AppState;

GtkBuilder *load_ui(AppState *state);

void main_view_setup(gulong socket_id, AppState *state);

#endif // KORADAPP_H_
