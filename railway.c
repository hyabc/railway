#include <gtk/gtk.h>
#include <gst/gst.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include "railwaylib.h"
#include "railwaymusic.h"

GtkBuilder *builder;

void pause_button_trigger_cb(GtkWidget *widget, void*) {
	bool play_state = music_pause_trigger();
	if (play_state) {
		gtk_button_set_label(GTK_BUTTON(widget), "⏸");
	} else {
		gtk_button_set_label(GTK_BUTTON(widget), "▶");
	}
}

void func() {
	g_print("Hello World\n");
}

void play_song_cb(GtkWidget *widget, song_type *current_song) {
	music_play(current_song->filename);
}

void update_songs_cb(GtkWidget *widget, album_type *current_album) {
	//Update song list
	destroy_song_list();
	generate_song_list(current_album);

	//Reset song widget
	GtkWidget *songs = GTK_WIDGET(gtk_builder_get_object(builder, "songs"));
	gtk_container_foreach(GTK_CONTAINER(songs), (GtkCallback)(gtk_widget_destroy), NULL);
	for (int i = 0;i < song_count;i++) {
		//Create a button
		GtkWidget *button = gtk_button_new();
		gtk_container_add(GTK_CONTAINER(songs), button);
		gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
		gtk_widget_set_visible(button, TRUE);
		gtk_widget_set_size_request(button, -1, 48);
		g_signal_connect(button, "clicked", G_CALLBACK(play_song_cb), song_array[i]);

		//Create a label in the button
		GtkWidget *label = gtk_label_new(song_array[i]->song_name);
		gtk_container_add(GTK_CONTAINER(button), label);
		gtk_label_set_xalign(GTK_LABEL(label), 0);
		gtk_widget_set_visible(label, TRUE);
	}
}

GTask **album_image_tasks;
size_t album_image_task_count;

void album_image_draw_cb(GtkWidget *widget, void*, void*) {
	album_type *current_album = album_array[album_image_task_count];

	//Create image path
	char image_path_buffer[PATH_LENGTH_MAX];
	strcpy(image_path_buffer, album_cache_path);
	strcat(image_path_buffer, "/");
	strcat(image_path_buffer, current_album->album_name);
	strcat(image_path_buffer, ".jpg");

	//Create pixbuf
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(image_path_buffer, 160, 160, FALSE, NULL);
	if (pixbuf != NULL) {
		//Create an image in the button
		GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
		gtk_button_set_image(GTK_BUTTON(widget), image);
		gtk_widget_set_visible(image, TRUE);
	}

	//Prepare next image generation
	album_image_task_count++;
	if (album_image_task_count < album_count) {
		g_task_run_in_thread(album_image_tasks[album_image_task_count], (GTaskThreadFunc)(generate_album_button_image));
	} else {
		for (int i = 0;i < album_count;i++) {
			g_object_unref(album_image_tasks[i]);
		}
		free(album_image_tasks);
	}
}

void init_albums() {
	//Init gtask array
	album_image_tasks = malloc(sizeof(GTask*) * album_count);
	if (album_image_tasks == NULL) {
		fprintf(stderr, "Insufficient memory\n");
		exit(1);
	}

	//Create buttons
	GtkWidget *albums_widget = GTK_WIDGET(gtk_builder_get_object(builder, "albums"));
	for (int i = 0;i < album_count;i++) {
		//Create a box
		GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_container_add(GTK_CONTAINER(albums_widget), box);
		gtk_widget_set_visible(box, TRUE);

		//Create a button in the box
		GtkWidget *button = gtk_button_new();
		gtk_container_add(GTK_CONTAINER(box), button);
		gtk_widget_set_size_request(button, 160, 160);
		gtk_widget_set_margin_start(button, 5);
		gtk_widget_set_margin_end(button, 5);
		gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
		gtk_widget_set_visible(button, TRUE);
		g_signal_connect(button, "clicked", G_CALLBACK(update_songs_cb), album_array[i]);

		//Create a label in the box
		GtkWidget *label = gtk_label_new(album_array[i]->album_name);
		gtk_container_add(GTK_CONTAINER(box), label);
		gtk_label_set_max_width_chars(GTK_LABEL(label), 15);
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_widget_set_visible(label, TRUE);
		
		//Create task for each album
		album_image_tasks[i] = g_task_new(button, NULL, (GAsyncReadyCallback)(album_image_draw_cb), NULL);
		g_task_set_task_data(album_image_tasks[i], album_array[i], NULL);
	}

	//Launch first album drawing thread
	album_image_task_count = 0;
	g_task_run_in_thread(album_image_tasks[0], (GTaskThreadFunc)(generate_album_button_image));
}

void init_window() {
	GObject *window;
	window = gtk_builder_get_object(builder, "window");
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_window_set_title(GTK_WINDOW(window), "Project Railway");
}

void control_volume_cb(GtkWidget *widget, gpointer data) {
	music_volume(gtk_adjustment_get_value(GTK_ADJUSTMENT(widget)));
}

void init_control() {
	GObject *button;
	button = gtk_builder_get_object(builder, "next");
	g_signal_connect(button, "clicked", G_CALLBACK(func), NULL);
	button = gtk_builder_get_object(builder, "previous");
	g_signal_connect(button, "clicked", G_CALLBACK(func), NULL);
	button = gtk_builder_get_object(builder, "play");
	g_signal_connect(button, "clicked", G_CALLBACK(pause_button_trigger_cb), NULL);

	GObject *adj;
	adj = gtk_builder_get_object(builder, "volume_adjustment");
	g_signal_connect(adj, "value_changed", G_CALLBACK(control_volume_cb), NULL);
}

int main(int argc, char* argv[]) {
	gtk_init(&argc, &argv);
	gst_init(&argc, &argv);

	builder = gtk_builder_new();
	if (gtk_builder_add_from_file(builder, "railway.ui", NULL) == 0) {
		g_print("builder loading error\n");
		return 1;
	}

	init_music();
	init_library();
	init_window();
	init_control();
	generate_album_list();
	init_albums();

	gtk_main();

	destroy_album_list();
	destroy_song_list();

	destroy_music();

	g_object_unref(builder);
	return 0;
}
