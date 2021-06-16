#include <gst/gst.h>
#include <gst/player/player.h>
#include <stdbool.h>
#include "railwaylib.h"
#include "railwaymusic.h"

GstPlayer *music_player;
bool music_play_state;

void music_play(const char* filename) {
	gst_player_stop(music_player);

	char song_path_buffer[PATH_LENGTH_MAX];
	strcpy(song_path_buffer, "file:");
	strcat(song_path_buffer, filename);

	gst_player_set_uri(music_player, song_path_buffer);
	gst_player_play(music_player);
	music_play_state = true;
}

void music_pause_trigger() {
	if (music_play_state == false) {
		gst_player_play(music_player);
		music_play_state = true;
	} else {
		gst_player_pause(music_player);
		music_play_state = false;
	}
}

void init_music() {
	music_player = gst_player_new(NULL, NULL);
	music_play_state = false;
}
