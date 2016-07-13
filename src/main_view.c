#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <korad.h>
#include <string.h>
#include "koradapp.h"

void
update_voltage_display(AppState *state)
{
    gtk_entry_set_text(state->voltage_entry[0], g_strdup_printf("%u", state->voltage[0]));
    gtk_entry_set_text(state->voltage_entry[1], g_strdup_printf("%u", state->voltage[1]));
    gtk_entry_set_text(state->voltage_entry[2], g_strdup_printf("%u", state->voltage[2]));
    gtk_entry_set_text(state->voltage_entry[3], g_strdup_printf("%u", state->voltage[3]));
}

void
update_current_display(AppState *state)
{
    gtk_entry_set_text(state->current_entry[0], g_strdup_printf("%u", state->current[0]));
    gtk_entry_set_text(state->current_entry[1], g_strdup_printf("%u", state->current[1]));
    gtk_entry_set_text(state->current_entry[2], g_strdup_printf("%u", state->current[2]));
    gtk_entry_set_text(state->current_entry[3], g_strdup_printf("%u", state->current[3]));
}

void
on_update_voltage(KoradDevice *d, KoradCommand *c, void *user_data)
{
    AppState *state = (AppState*) user_data;
    float val = *((float*)c->result);
    gchar *tmp = g_strdup_printf("%05.2f", val);
    g_info("Got voltage: %05.2f", val);
    state->voltage[0] = tmp[0] - '0';
    state->voltage[1] = tmp[1] - '0';
    state->voltage[2] = tmp[3] - '0';
    state->voltage[3] = tmp[4] - '0';

    update_voltage_display(state);
}

void
on_update_current(KoradDevice *d, KoradCommand *c, void *user_data)
{
    AppState *state = (AppState*) user_data;
    float val = *((float*)c->result);
    gchar *tmp = g_strdup_printf("%05.3f", val);
    g_info("Got current: %05.3f", val);
    state->current[0] = tmp[0];
    state->current[1] = tmp[2];
    state->current[2] = tmp[3];
    state->current[3] = tmp[4];

    update_current_display(state);
}

void
on_output_toggle(GtkToggleButton *button, gpointer user_data)
{
    AppState *state = (AppState*) user_data;
    korad_output(state->device, gtk_toggle_button_get_active(button));
    korad_get_maximum_voltage(state->device, on_update_voltage, state);
}

void
on_ovp_toggled(GtkToggleButton *button, gpointer user_data)
{
    AppState *state = (AppState*) user_data;
    korad_ovp(state->device, gtk_toggle_button_get_active(button));
}

void
on_ocp_toggled(GtkToggleButton *button, gpointer user_data)
{
    AppState *state = (AppState*) user_data;
    korad_ocp(state->device, gtk_toggle_button_get_active(button));
}

void
on_save_toggled(GtkToggleButton *button, gpointer user_data)
{
    AppState *state = (AppState*) user_data;
    state->save = gtk_toggle_button_get_active(button);
}

void
on_memory_button_clicked(GtkButton *button, gpointer user_data, guint slot)
{
    AppState *state = (AppState*) user_data;
    if (state->save)
    {
        korad_save(state->device, slot);
        gtk_toggle_button_set_active(state->save_toggle, FALSE);
    }
    else
        korad_recall(state->device, slot);
}

void
on_m1_clicked(GtkButton *button, gpointer user_data)
{

    on_memory_button_clicked(button, user_data, 1);
}

void
on_m2_clicked(GtkButton *button, gpointer user_data)
{

    on_memory_button_clicked(button, user_data, 2);
}

void
on_m3_clicked(GtkButton *button, gpointer user_data)
{

    on_memory_button_clicked(button, user_data, 3);
}

void
on_m4_clicked(GtkButton *button, gpointer user_data)
{

    on_memory_button_clicked(button, user_data, 4);
}

void
on_m5_clicked(GtkButton *button, gpointer user_data)
{

    on_memory_button_clicked(button, user_data, 5);
}

void
on_voltage_add_ten(GtkButton *button, gpointer user_data)
{
    AppState *state = (AppState*) user_data;

    state->voltage[0] = (state->voltage[0] + 1) % 4;
    if (state->voltage[0] == 3)
    {
        state->voltage[1] = 0;
        state->voltage[2] = 0;
        state->voltage[3] = 0;
    }

    update_voltage_display(state);

    gchar *volt_str = g_strdup_printf("%c%c.%c%c", state->voltage[0], state->voltage[1], state->voltage[2], state->voltage[3]);
    korad_set_voltage_str(state->device, volt_str);
    g_free(volt_str);
}

void
on_voltage_substract_ten(GtkButton *button, gpointer user_data)
{
    AppState *state = (AppState*) user_data;

    if (state->voltage[0] < 0)
        state->voltage[0] = 3;
    else
        state->voltage[0] = (state->voltage[0] - 1);

    update_voltage_display(state);

    gchar *volt_str = g_strdup_printf("%c%c.%c%c", state->voltage[0], state->voltage[1], state->voltage[2], state->voltage[3]);
    korad_set_voltage_str(state->device, volt_str);
    g_free(volt_str);
}

void
main_view_setup(gulong socket_id, AppState *state)
{
    state->voltage[4] = 0;
    state->current[4] = 0;

    GtkBuilder *builder = load_ui(state);
    GtkContainer *window = GTK_CONTAINER(gtk_builder_get_object(builder, "Main Window"));
    GtkWidget *box = GTK_WIDGET(gtk_builder_get_object(builder, "Outer Box"));
    g_object_ref(box);
    GtkWidget *plug = gtk_plug_new(socket_id);

    gtk_container_remove(window, box);
    gtk_container_add(GTK_CONTAINER(plug), box);
    gtk_widget_show_all(GTK_WIDGET(plug));

    state->main_builder = builder;

    state->voltage_entry[0] = GTK_ENTRY(gtk_builder_get_object(builder, "Voltage Display Ten"));
    state->voltage_entry[1] = GTK_ENTRY(gtk_builder_get_object(builder, "Voltage Display One"));
    state->voltage_entry[2] = GTK_ENTRY(gtk_builder_get_object(builder, "Voltage Display Tenth"));
    state->voltage_entry[3] = GTK_ENTRY(gtk_builder_get_object(builder, "Voltage Display Hundredth"));

    state->current_entry[0] = GTK_ENTRY(gtk_builder_get_object(builder, "Current Display One"));
    state->current_entry[1] = GTK_ENTRY(gtk_builder_get_object(builder, "Current Display Tenth"));
    state->current_entry[2] = GTK_ENTRY(gtk_builder_get_object(builder, "Current Display Hundredth"));
    state->current_entry[3] = GTK_ENTRY(gtk_builder_get_object(builder, "Current Display Thousandth"));

    state->save_toggle = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "Save"));

    korad_get_maximum_voltage(state->device, on_update_voltage, state);
    korad_get_maximum_current(state->device, on_update_current, state);
}