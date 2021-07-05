#include <gst/gst.h>
#include <gst/player/player.h>
#include <stdbool.h>
#include "railway.h"
#include "railwaylib.h"
#include "railwayplaylist.h"
#include "railwaymusic.h"

GstPlayer *music_player, *music_player_last;
bool music_play_state;
int music_duration;

void music_play(song_type *current_song) {
	//Remove music_player
	if (music_player != NULL) gst_player_stop(music_player);
	if (music_player_last != NULL) g_clear_object(&music_player_last);
	music_player_last = music_player;

	//Create new gst_player object
	music_player = gst_player_new(NULL, NULL);
	music_duration = current_song->duration;
	g_signal_connect(music_player, "end-of-stream", G_CALLBACK(playlist_next), NULL);
	g_signal_connect(music_player, "position-updated", G_CALLBACK(music_position_update_cb), &music_duration);

	//Prepare song path
	char *song_path_buffer = malloc(strlen("file://") + strlen(current_song->filename) + 1);
	if (song_path_buffer == NULL) {
		fprintf(stderr, "Insufficient memory\n");
		exit(1);
	}
	strcpy(song_path_buffer, "file://");
	strcat(song_path_buffer, current_song->filename);

	//Set URI and play
	gst_player_set_uri(music_player, song_path_buffer);
	free(song_path_buffer);
	gst_player_play(music_player);
	music_play_state = true;
}

void music_pause_trigger() {
	if (music_play_state) {
		gst_player_pause(music_player);
		music_play_state = false;
	} else {
		gst_player_play(music_player);
		music_play_state = true;
	}
}

bool music_is_playing() {
	return music_play_state;
}

void music_volume(double value) {
	gst_player_set_volume(music_player, value);
}

void init_music() {
	music_player = NULL;
	music_player_last = NULL;
	music_play_state = false;
	music_duration = 0;
}

void destroy_music() {
	if (music_player_last != NULL) g_clear_object(&music_player_last);
	if (music_player != NULL) g_clear_object(&music_player);
}
