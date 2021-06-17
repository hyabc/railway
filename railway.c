#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/player/player.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include "railway.h"
#include "railwaylib.h"
#include "railwaymusic.h"
#include "railwayplaylist.h"

GtkBuilder *builder;

void music_position_update_cb(GstPlayer*, GstClockTime, void*) {
	//Get time
	int position_sec = GST_TIME_AS_SECONDS(music_position());
	int duration_sec = GST_TIME_AS_SECONDS(music_duration());
	double progress = (double)(position_sec) / (double)(duration_sec);

	//Update playback
	GObject *playback_adj;
	playback_adj = gtk_builder_get_object(builder, "playback_adjustment");
	gtk_adjustment_set_value(GTK_ADJUSTMENT(playback_adj), progress);

	//Prepare duration text
	char duration_buffer[NAME_LENGTH_MAX];
	sprintf(duration_buffer, "%02d:%02d/%02d:%02d", position_sec / 60, position_sec % 60, duration_sec / 60, duration_sec % 60);

	//Set song_position_label
	GtkWidget *song_position_label = GTK_WIDGET(gtk_builder_get_object(builder, "song_position_label"));
	gtk_label_set_text(GTK_LABEL(song_position_label), duration_buffer);
}

void music_position_modify_cb(GtkWidget* widget, void*) {
	//Get time
	int position_sec = GST_TIME_AS_SECONDS(music_position());
	int duration_sec = GST_TIME_AS_SECONDS(music_duration());
	double progress = (double)(position_sec) / (double)(duration_sec);
	double new_progress = gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));

	//Update position if needed
	if (fabs(progress - new_progress) > 0.01) {
		music_seek(new_progress * music_duration());
	}
}

void pause_button_trigger_cb(GtkWidget *widget, void*) {
	bool play_state = music_pause_trigger();
	if (play_state) {
		gtk_button_set_label(GTK_BUTTON(widget), "⏸");
	} else {
		gtk_button_set_label(GTK_BUTTON(widget), "▶");
	}
}

void play_song(song_type *current_song) {
	music_play(current_song->filename);

	//Prepare text for song_info_label
	char song_info_buffer[NAME_LENGTH_MAX];
	strcpy(song_info_buffer, "");
	if (current_song->tag_title != NULL) {
		strcat(song_info_buffer, current_song->tag_title);
	} else {
		strcat(song_info_buffer, "unknown");
	}
	strcat(song_info_buffer, " by ");
	if (current_song->tag_artist != NULL) {
		strcat(song_info_buffer, current_song->tag_artist);
	} else {
		strcat(song_info_buffer, "unknown");
	}

	//Set song_info_label
	GtkWidget *song_info_label = GTK_WIDGET(gtk_builder_get_object(builder, "song_info_label"));
	gtk_label_set_text(GTK_LABEL(song_info_label), song_info_buffer);

	//Create image path
	char image_path_buffer[PATH_LENGTH_MAX];
	strcpy(image_path_buffer, album_cache_path);
	strcat(image_path_buffer, "/");
	strcat(image_path_buffer, album_array[current_song->album_id]->album_name);
	strcat(image_path_buffer, ".jpg");

	//Create pixbuf
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(image_path_buffer, 80, 80, FALSE, NULL);
	if (pixbuf != NULL) {
		//Apply image to song_info_image
		GtkWidget *song_info_image = GTK_WIDGET(gtk_builder_get_object(builder, "song_info_image"));
		gtk_image_set_from_pixbuf(GTK_IMAGE(song_info_image), pixbuf);
		gtk_widget_set_visible(song_info_image, TRUE);
	}
}

void song_button_cb(GtkWidget *widget, song_type *current_song) {
	playlist_play(current_song);
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
		g_signal_connect(button, "clicked", G_CALLBACK(song_button_cb), song_array[i]);

		//Create a label in the button
		GtkWidget *label = gtk_label_new(song_array[i]->song_name);
		gtk_container_add(GTK_CONTAINER(button), label);
		gtk_label_set_xalign(GTK_LABEL(label), 0);
		gtk_widget_set_visible(label, TRUE);
	}
}

size_t album_image_task_count;

void album_image_draw_cb(GtkWidget *widget, void*, GTask **album_image_tasks, void*) {
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
	GTask **album_image_tasks = malloc(sizeof(GTask*) * album_count);
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
		album_image_tasks[i] = g_task_new(button, NULL, (GAsyncReadyCallback)(album_image_draw_cb), album_image_tasks);
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

void control_next_cb(GtkWidget*, void*) {
	playlist_next();
}

void control_prev_cb(GtkWidget*, void*) {
	playlist_prev();
}

void init_control() {
	GObject *button;
	button = gtk_builder_get_object(builder, "next");
	g_signal_connect(button, "clicked", G_CALLBACK(control_next_cb), NULL);
	button = gtk_builder_get_object(builder, "previous");
	g_signal_connect(button, "clicked", G_CALLBACK(control_prev_cb), NULL);
	button = gtk_builder_get_object(builder, "play");
	g_signal_connect(button, "clicked", G_CALLBACK(pause_button_trigger_cb), NULL);

	GObject *vol_adj, *playback_adj;
	vol_adj = gtk_builder_get_object(builder, "volume_adjustment");
	g_signal_connect(vol_adj, "value_changed", G_CALLBACK(control_volume_cb), NULL);
	playback_adj = gtk_builder_get_object(builder, "playback_adjustment");
	g_signal_connect(playback_adj, "value_changed", G_CALLBACK(music_position_modify_cb), NULL);
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
	init_playlist();
	init_library();
	init_window();
	init_control();
	generate_album_list();
	init_albums();

	gtk_main();

	destroy_album_list();
	destroy_song_list();

	destroy_music();
	destroy_library();
	destroy_playlist();

	g_object_unref(builder);
	return 0;
}
