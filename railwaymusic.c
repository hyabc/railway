#include <gst/gst.h>
#include <gst/player/player.h>
#include <stdbool.h>
#include "railway.h"
#include "railwaylib.h"
#include "railwayplaylist.h"
#include "railwaymusic.h"

GstPlayer *music_player;
GstPlayerState music_play_state;

void music_play(const char* filename) {
	gst_player_stop(music_player);

	char song_path_buffer[PATH_LENGTH_MAX];
	strcpy(song_path_buffer, "file://");
	strcat(song_path_buffer, filename);

	gst_player_set_uri(music_player, song_path_buffer);
	gst_player_play(music_player);
}

void music_state_changed_cb(GstPlayer*, GstPlayerState state, GtkWidget *widget) {
	music_play_state = state;
	GtkWidget *img;
	if (music_play_state == GST_PLAYER_STATE_PLAYING) {
		img = gtk_image_new_from_icon_name("media-playback-pause-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
	} else {
		img = gtk_image_new_from_icon_name("media-playback-start-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
	}
	gtk_button_set_image(GTK_BUTTON(widget), img);
}

void music_pause_trigger() {
	if (music_play_state == GST_PLAYER_STATE_PLAYING) {
		gst_player_pause(music_player);
	} else {
		gst_player_play(music_player);
	}
}

void music_volume(double value) {
	gst_player_set_volume(music_player, value);
}

GstClockTime music_duration() {
	return gst_player_get_duration(music_player);
}

GstClockTime music_position() {
	return gst_player_get_position(music_player);
}

void music_seek(GstClockTime time) {
	gst_player_seek(music_player, time);
}

void init_music(GtkWidget *button) {
	music_player = gst_player_new(NULL, NULL);

	g_signal_connect(music_player, "position-updated", G_CALLBACK(music_position_update_cb), NULL);
	g_signal_connect(music_player, "end-of-stream", G_CALLBACK(playlist_next), NULL);
	g_signal_connect(music_player, "state-changed", G_CALLBACK(music_state_changed_cb), button);
}

void destroy_music() {
	g_object_unref(music_player);
}
