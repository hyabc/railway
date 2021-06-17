#include "railwaylib.h"
#include "railway.h"

song_type **playlist_array;
int playlist_count;
int playlist_ptr;

void generate_playlist() {
	playlist_count = song_count;
	playlist_array = malloc(song_count * sizeof(song_type*));
	if (playlist_array == NULL) {
		fprintf(stderr, "Insufficient memory\n");
		exit(1);
	}
	for (int i = 0;i < song_count;i++) {
		playlist_array[i] = malloc(sizeof(song_type));
		if (playlist_array[i] == NULL) {
			fprintf(stderr, "Insufficient memory\n");
			exit(1);
		}
		memcpy(playlist_array[i], song_array[i], sizeof(song_type));

		playlist_array[i]->filename = malloc(strlen(song_array[i]->filename) + 1);
		playlist_array[i]->song_name = malloc(strlen(song_array[i]->song_name) + 1);
		if (playlist_array[i]->filename == NULL || playlist_array[i]->song_name == NULL) {
			fprintf(stderr, "Insufficient memory\n");
			exit(1);
		}
		strcpy(playlist_array[i]->filename, song_array[i]->filename);
		strcpy(playlist_array[i]->song_name, song_array[i]->song_name);

		if (song_array[i]->tag_title != NULL) {
			playlist_array[i]->tag_title = malloc(strlen(song_array[i]->tag_title) + 1);
			if (playlist_array[i]->tag_title == NULL) {fprintf(stderr, "Insufficient memory\n");exit(1);}
			strcpy(playlist_array[i]->tag_title, song_array[i]->tag_title);
		} else {
			playlist_array[i]->tag_title = NULL;
		}
		if (song_array[i]->tag_album != NULL) {
			playlist_array[i]->tag_album = malloc(strlen(song_array[i]->tag_album) + 1);
			if (playlist_array[i]->tag_album == NULL) {fprintf(stderr, "Insufficient memory\n");exit(1);}
			strcpy(playlist_array[i]->tag_album, song_array[i]->tag_album);
		} else {
			playlist_array[i]->tag_album = NULL;
		}
		if (song_array[i]->tag_artist != NULL) {
			playlist_array[i]->tag_artist = malloc(strlen(song_array[i]->tag_artist) + 1);
			if (playlist_array[i]->tag_artist == NULL) {fprintf(stderr, "Insufficient memory\n");exit(1);}
			strcpy(playlist_array[i]->tag_artist, song_array[i]->tag_artist);
		} else {
			playlist_array[i]->tag_artist = NULL;
		}
	}
}

void init_playlist() {
	playlist_array = NULL;
	playlist_count = 0;
	playlist_ptr = 0;
}

void destroy_playlist() {
	for (int i = 0;i < playlist_count;i++) {
		free(playlist_array[i]->filename);
		free(playlist_array[i]->song_name);
		if (playlist_array[i]->tag_title != NULL) free(playlist_array[i]->tag_title);
		if (playlist_array[i]->tag_album != NULL) free(playlist_array[i]->tag_album);
		if (playlist_array[i]->tag_artist != NULL) free(playlist_array[i]->tag_artist);
		free(playlist_array[i]);
	}
	free(playlist_array);
	playlist_array = NULL;
	playlist_count = 0;
	playlist_ptr = 0;
}

void playlist_play(song_type *current_song) {
	//Copy playlist
	destroy_playlist();
	generate_playlist();

	//Set position and play
	playlist_ptr = current_song->id;
	play_song(playlist_array[playlist_ptr]);
}

void playlist_next() {
	if (playlist_ptr + 1 < playlist_count) {
		playlist_ptr++;
		play_song(playlist_array[playlist_ptr]);
	}
}

void playlist_prev() {
	if (playlist_ptr - 1 >= 0) {
		playlist_ptr--;
		play_song(playlist_array[playlist_ptr]);
	}
}
