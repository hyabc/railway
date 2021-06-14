#include <gtk/gtk.h>

void func(GtkWidget *widget, gpointer data) {
	g_print("Hello World\n");
}

char arr[100][100];

void init_albums(GtkBuilder* builder) {
	strcpy(arr[0], "QwQ");
	strcpy(arr[1], "QuQ");
	strcpy(arr[2], "QAQ");
	strcpy(arr[3], "QwQ");
	strcpy(arr[4], "QuQ");
	strcpy(arr[5], "QAQ");
	strcpy(arr[6], "QwQ");
	strcpy(arr[7], "QuQ");
	strcpy(arr[8], "QAQ");
	strcpy(arr[9], "QAQ");
	GtkWidget *albums = GTK_WIDGET(gtk_builder_get_object(builder, "albums"));
	for (int i = 0;i < 10;i++) {
		GtkWidget *button = gtk_button_new_with_label(arr[i]);
		gtk_container_add(GTK_CONTAINER(albums), button);
		gtk_widget_set_size_request(button, 120, 120);
		gtk_widget_set_margin_start(button, 20);
		gtk_widget_set_margin_end(button, 20);
		gtk_widget_set_margin_top(button, 20);
		gtk_widget_set_margin_bottom(button, 20);
		gtk_widget_set_visible(button, TRUE);
	}
}

void init_songs(GtkBuilder* builder) {
	strcpy(arr[0], "QwQ");
	strcpy(arr[1], "QuQ");
	strcpy(arr[2], "QAQ");
	strcpy(arr[3], "QwQ");
	strcpy(arr[4], "QuQ");
	strcpy(arr[5], "QAQ");
	strcpy(arr[6], "QwQ");
	strcpy(arr[7], "QuQ");
	strcpy(arr[8], "QAQ");
	strcpy(arr[9], "QAQ");
	GtkWidget *songs = GTK_WIDGET(gtk_builder_get_object(builder, "songs"));
	gtk_container_foreach(GTK_CONTAINER(songs), (GtkCallback)(gtk_widget_destroy), NULL);
	for (int i = 0;i < 10;i++) {
		GtkWidget *button = gtk_button_new_with_label(arr[i]);
		gtk_container_add(GTK_CONTAINER(songs), button);
		gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
		gtk_widget_set_visible(button, TRUE);
	}
}

void init_signal(GtkBuilder* builder) {
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

	GtkBuilder* builder = gtk_builder_new();
	if (gtk_builder_add_from_file(builder, "railway.ui", NULL) == 0) {
		g_print("builder loading error\n");
		return 1;
	}

	init_albums(builder);
	init_songs(builder);
	init_songs(builder);
	init_signal(builder);

	gtk_main();

	return 0;
}
