#include <gtk/gtk.h>
#include "railwaylib.h"

GtkBuilder *builder;

void func() {
	g_print("Hello World\n");
}

void song_button_callback(GtkWidget *widget, gpointer data) {
	song_type *current_song = data;
	g_print("%s\n", current_song->song_name);
}

void init_songs() {
	GtkWidget *songs = GTK_WIDGET(gtk_builder_get_object(builder, "songs"));
	gtk_container_foreach(GTK_CONTAINER(songs), (GtkCallback)(gtk_widget_destroy), NULL);
}

void update_songs(const char* filename) {
	destroy_song_list();
	generate_song_list(filename);
	GtkWidget *songs = GTK_WIDGET(gtk_builder_get_object(builder, "songs"));
	gtk_container_foreach(GTK_CONTAINER(songs), (GtkCallback)(gtk_widget_destroy), NULL);
	for (int i = 0;i < song_count;i++) {
		GtkWidget *button = gtk_button_new();
		gtk_container_add(GTK_CONTAINER(songs), button);
		gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
		gtk_widget_set_visible(button, TRUE);
		g_signal_connect(button, "clicked", G_CALLBACK(song_button_callback), song_array[i]);

		GtkWidget *label = gtk_label_new(song_array[i]->song_name);
		gtk_container_add(GTK_CONTAINER(button), label);
		gtk_label_set_xalign(GTK_LABEL(label), 0);
		gtk_widget_set_visible(label, TRUE);
	}
}

void album_button_callback(GtkWidget *widget, gpointer data) {
	album_type *current_album = data;
	update_songs(current_album->filename);
}

void init_albums() {
	GtkWidget *albums_widget = GTK_WIDGET(gtk_builder_get_object(builder, "albums"));
	for (int i = 0;i < album_count;i++) {
		GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_container_add(GTK_CONTAINER(albums_widget), box);
		gtk_widget_set_visible(box, TRUE);

		GtkWidget *button = gtk_button_new();
		gtk_container_add(GTK_CONTAINER(box), button);
		gtk_widget_set_size_request(button, 160, 160);
		gtk_widget_set_margin_start(button, 5);
		gtk_widget_set_margin_end(button, 5);
		gtk_widget_set_visible(button, TRUE);
		g_signal_connect(button, "clicked", G_CALLBACK(album_button_callback), album_array[i]);

		GtkWidget *label = gtk_label_new(album_array[i]->album_name);
		gtk_container_add(GTK_CONTAINER(box), label);
		gtk_label_set_max_width_chars(GTK_LABEL(label), 15);
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_widget_set_visible(label, TRUE);
	}
}

void init_signal() {
	GObject *window, *button;
	window = gtk_builder_get_object(builder, "window");
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	button = gtk_builder_get_object(builder, "next");
	g_signal_connect(button, "clicked", G_CALLBACK(func), NULL);
	button = gtk_builder_get_object(builder, "previous");
	g_signal_connect(button, "clicked", G_CALLBACK(func), NULL);
	button = gtk_builder_get_object(builder, "play");
	g_signal_connect(button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

}

int main(int argc, char* argv[]) {
	gtk_init(&argc, &argv);

	builder = gtk_builder_new();
	if (gtk_builder_add_from_file(builder, "railway.ui", NULL) == 0) {
		g_print("builder loading error\n");
		return 1;
	}

	init_library();
	generate_album_list();
	init_albums();
	init_songs();
	init_songs();
	init_signal();

	gtk_main();

	destroy_album_list();
	destroy_song_list();
	return 0;
}
