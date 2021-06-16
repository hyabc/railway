#include <gst/gst.h>
#include <gst/player/player.h>
#include <stdbool.h>
#include "railway.h"
#include "railwaylib.h"
#include "railwaymusic.h"

GstPlayer *music_player;
bool music_play_state;

void music_play(const char* filename) {
	gst_player_stop(music_player);

	char song_path_buffer[PATH_LENGTH_MAX];
	strcpy(song_path_buffer, "file://");
	strcat(song_path_buffer, filename);

	gst_player_set_uri(music_player, song_path_buffer);
	gst_player_play(music_player);
	music_play_state = true;
}

bool music_pause_trigger() {
	if (music_play_state == false) {
		gst_player_play(music_player);
		music_play_state = true;
	} else {
		gst_player_pause(music_player);
		music_play_state = false;
	}
	return music_play_state;
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

void init_music() {
	music_player = gst_player_new(NULL, NULL);
	music_play_state = false;

	g_signal_connect(music_player, "position-updated", G_CALLBACK(music_position_update_cb), NULL);
}

void destroy_music() {
	g_object_unref(music_player);
}
