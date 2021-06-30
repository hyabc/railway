#include <gst/gst.h>
#include <gst/player/player.h>
#include <stdbool.h>
#include "railway.h"
#include "railwaylib.h"
#include "railwayplaylist.h"
#include "railwaymusic.h"

GstPlayer *music_player;
GstPlayerState music_play_state;
int music_duration;

void music_play(song_type *current_song) {
	music_duration = current_song->duration;

	char song_path_buffer[PATH_LENGTH_MAX];
	strcpy(song_path_buffer, "file://");
	strcat(song_path_buffer, current_song->filename);

	gst_player_set_uri(music_player, song_path_buffer);
	gst_player_play(music_player);
}

void music_state_changed_cb(void*, GstPlayerState state, void*) {
	music_play_state = state;
}

void music_pause_trigger() {
	if (music_is_playing()) {
		gst_player_pause(music_player);
	} else {
		gst_player_play(music_player);
	}
}

bool music_is_playing() {
	return music_play_state == GST_PLAYER_STATE_PLAYING;
}

void music_volume(double value) {
	gst_player_set_volume(music_player, value);
}

void init_music() {
	music_player = gst_player_new(NULL, NULL);
	music_play_state = GST_PLAYER_STATE_STOPPED;
	music_duration = 0;

	g_signal_connect(music_player, "position-updated", G_CALLBACK(music_position_update_cb), &music_duration);
	g_signal_connect(music_player, "end-of-stream", G_CALLBACK(playlist_next), &music_duration);
	g_signal_connect(music_player, "state-changed", G_CALLBACK(music_state_changed_cb), NULL);
}

void destroy_music() {
	g_object_unref(music_player);
}
