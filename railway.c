#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "railwaylib.h"

GtkBuilder *builder;

void func() {
	g_print("Hello World\n");
}

void play_song(GtkWidget *widget, song_type *current_song) {
	g_print("%s\n", current_song->song_name);

	pid_t pid;
	int ret;
	if ((pid = fork()) == 0) {
/*		int null_fd = open("/dev/null", O_WRONLY);
		dup2(null_fd, 1);
		dup2(null_fd, 2);*/
		printf("%s\n", current_song->filename);
		execl("/usr/bin/ffplay", "-nostdin", current_song->filename, NULL);
	}
}

void update_songs(GtkWidget *widget, album_type *current_album) {
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
		g_signal_connect(button, "clicked", G_CALLBACK(play_song), song_array[i]);

		//Create a label in the button
		GtkWidget *label = gtk_label_new(song_array[i]->song_name);
		gtk_container_add(GTK_CONTAINER(button), label);
		gtk_label_set_xalign(GTK_LABEL(label), 0);
		gtk_widget_set_visible(label, TRUE);
	}
}

GTask **album_image_tasks;
size_t album_image_task_count;

void album_image_draw(GtkWidget *widget, void*, void*) {
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
		g_signal_connect(button, "clicked", G_CALLBACK(update_songs), album_array[i]);

		//Create a label in the box
		GtkWidget *label = gtk_label_new(album_array[i]->album_name);
		gtk_container_add(GTK_CONTAINER(box), label);
		gtk_label_set_max_width_chars(GTK_LABEL(label), 15);
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_widget_set_visible(label, TRUE);
		
		//Create task for each album
		album_image_tasks[i] = g_task_new(button, NULL, (GAsyncReadyCallback)(album_image_draw), NULL);
		g_task_set_task_data(album_image_tasks[i], album_array[i], NULL);
	}

	//Launch first album drawing thread
	album_image_task_count = 0;
	g_task_run_in_thread(album_image_tasks[0], (GTaskThreadFunc)(generate_album_button_image));
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
	init_signal();
	generate_album_list();
	init_albums();

	gtk_main();

	destroy_album_list();
	destroy_song_list();
	return 0;
}
