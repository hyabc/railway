#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include "railwaylib.h"
#include <gtk/gtk.h>

#define PATH_LENGTH_MAX 100
#define NAME_LENGTH_MAX 100

char library_path[PATH_LENGTH_MAX], path_buffer[PATH_LENGTH_MAX];
char artist_buffer[NAME_LENGTH_MAX], album_buffer[NAME_LENGTH_MAX];

void init_library() {
	// Create configuration path
	strcpy(path_buffer, getenv("HOME"));
	strcat(path_buffer, "/.railway");

	//Read configuration LIBRARY_PATH using GTK+ library
	GKeyFile *keyfile = g_key_file_new();
	gchar *value;
	if (g_key_file_load_from_file(keyfile, path_buffer, G_KEY_FILE_NONE, NULL) == FALSE) {
		fprintf(stderr, "read configuration failed\n");
		exit(1);
	}
	if ((value = g_key_file_get_string(keyfile, "railway", "LIBRARY_PATH", NULL)) == NULL) {
		fprintf(stderr, "configuration error\n");
		exit(1);
	}
	strcpy(library_path, value);
	g_key_file_free(keyfile);
}

void generate_album_list() {
	struct dirent *artist_dp, *album_dp;
	DIR *artist_dir = opendir(library_path);
	if (artist_dir == NULL) {
		fprintf(stderr, "open artist directory failed\n");
		exit(1);
	}
	//Iterate over artist directory
	while ((artist_dp = readdir(artist_dir)) != NULL) {
		strcpy(album_buffer, artist_dp->d_name);
		if (strcmp(album_buffer, ".") == 0 || strcmp(album_buffer, "..") == 0) continue;
		strcpy(path_buffer, library_path);
		strcat(path_buffer, "/");
		strcat(path_buffer, artist_dp->d_name);
		printf("%s - %s\n", album_buffer, path_buffer);
	}
	closedir(artist_dir);
}

void generate_song_list() {
}

int main() {
	init_library();
	generate_album_list();
	return 0;
}
