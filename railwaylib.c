#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "railwaylib.h"

#define PATH_LENGTH_MAX 1000
#define NAME_LENGTH_MAX 1000

char library_path[PATH_LENGTH_MAX];

void init_library() {
	char config_path_buffer[PATH_LENGTH_MAX];
	// Create configuration path
	strcpy(config_path_buffer, getenv("HOME"));
	strcat(config_path_buffer, "/.railway");

	//Read configuration LIBRARY_PATH using GTK+ library
	GKeyFile *keyfile = g_key_file_new();
	gchar *value;
	if (g_key_file_load_from_file(keyfile, config_path_buffer, G_KEY_FILE_NONE, NULL) == FALSE) {
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
	char artist_path_buffer[PATH_LENGTH_MAX], album_path_buffer[PATH_LENGTH_MAX];
	char artist_name_buffer[NAME_LENGTH_MAX], album_name_buffer[NAME_LENGTH_MAX];
	DIR *artist_dir, *album_dir;
	struct stat path_stat;

	if ((artist_dir = opendir(library_path)) == NULL) {
		fprintf(stderr, "open artist directory failed\n");
		exit(1);
	}
	//Iterate over artist directory
	while ((artist_dp = readdir(artist_dir)) != NULL) {
		strcpy(artist_name_buffer, artist_dp->d_name);
		if (strcmp(artist_name_buffer, ".") == 0 || strcmp(artist_name_buffer, "..") == 0) continue;

		//Generate artist path
		strcpy(artist_path_buffer, library_path);
		strcat(artist_path_buffer, "/");
		strcat(artist_path_buffer, artist_dp->d_name);

		//Iterate over album directory
		if ((album_dir = opendir(artist_path_buffer)) == NULL) continue;
		while ((album_dp = readdir(album_dir)) != NULL) {
			strcpy(album_name_buffer, album_dp->d_name);
			if (strcmp(album_name_buffer, ".") == 0 || strcmp(album_name_buffer, "..") == 0) continue;

			//Generate album path
			strcpy(album_path_buffer, artist_path_buffer);
			strcat(album_path_buffer, "/");
			strcat(album_path_buffer, album_dp->d_name);

			//This should be a directory to be an album folder
			stat(album_path_buffer, &path_stat);
			if (!S_ISDIR(path_stat.st_mode)) continue;

			printf("%s - %s\n", artist_name_buffer, album_name_buffer);
		}
		closedir(album_dir);
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
