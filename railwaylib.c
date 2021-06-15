#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "railwaylib.h"

#define PATH_LENGTH_MAX 1000
#define NAME_LENGTH_MAX 1000

char library_path[PATH_LENGTH_MAX], album_cache_path[PATH_LENGTH_MAX];
album_type *album_list, **album_array;
song_type *song_list, **song_array;
size_t album_count, song_count;

void init_library() {
	// Create configuration path
	char config_path_buffer[PATH_LENGTH_MAX];
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
	if ((value = g_key_file_get_string(keyfile, "railway", "ALBUM_CACHE_PATH", NULL)) == NULL) {
		fprintf(stderr, "configuration error\n");
		exit(1);
	}
	strcpy(album_cache_path, value);
	g_key_file_free(keyfile);

	album_array = NULL;
	song_array = NULL;
}

void generate_album_list() {
	//Init album list
	album_list = NULL;
	album_count = 0;
	album_type* album_last = NULL;

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

			//This should be a directory if it is an album folder
			stat(album_path_buffer, &path_stat);
			if (!S_ISDIR(path_stat.st_mode)) continue;

			//Create new album item
			album_type *current_album = malloc(sizeof(album_type));
			if (current_album == NULL) {
				fprintf(stderr, "Insufficient memory\n");
				exit(1);
			}
			current_album->next = NULL;
			current_album->id = album_count++;

			//Malloc memory
			current_album->filename = malloc(strlen(album_path_buffer) + 1);
			current_album->album_name = malloc(strlen(album_name_buffer) + 1);
			current_album->artist_name = malloc(strlen(artist_name_buffer) + 1);
			if (current_album->filename == NULL || current_album->album_name == NULL || current_album->artist_name == NULL) {
				fprintf(stderr, "Insufficient memory\n");
				exit(1);
			}
			strcpy(current_album->filename, album_path_buffer);
			strcpy(current_album->album_name, album_name_buffer);
			strcpy(current_album->artist_name, artist_name_buffer);

			if (album_last == NULL) {
				album_last = album_list = current_album;
			} else {
				album_last->next = current_album;
				album_last = current_album;
			}
		}
		closedir(album_dir);
	}
	closedir(artist_dir);

	//Generate array
	album_array = malloc(album_count * sizeof(album_type*));
	if (album_array == NULL) {
		fprintf(stderr, "Insufficient memory\n");
		exit(1);
	}
	album_type** album_iter = album_array;
	album_last = album_list;
	while (album_last != NULL) {
		*album_iter = album_last;
		album_iter++;
		album_last = album_last->next;
	}
/*
	char image_path_buffer[PATH_LENGTH_MAX], song_path_buffer[PATH_LENGTH_MAX];
	for (int i = 0;i < album_count;i++) {
		strcpy(image_path_buffer, album_cache_path);
		strcat(image_path_buffer, "/");
		strcat(image_path_buffer, album_array[i]->album_name);
		strcat(image_path_buffer, ".jpg");

		struct dirent *song_dp;
		DIR *song_dir;
		bool exist_song = false;
		if ((song_dir = opendir(album_array[i]->filename)) == NULL) {
			continue;
		}
		while ((song_dp = readdir(song_dir)) != NULL) {
			if (strcmp(song_dp->d_name, ".") == 0 || strcmp(song_dp->d_name, "..") == 0) continue;

			strcpy(song_path_buffer, album_array[i]->filename);
			strcat(song_path_buffer, "/");
			strcat(song_path_buffer, song_dp->d_name);

			stat(song_path_buffer, &path_stat);
			if (!S_ISREG(path_stat.st_mode)) continue;
			
			exist_song = true;
			break;
		}
		closedir(song_dir);

		if (!exist_song) continue;

		printf("write album %s\n", song_path_buffer);
		pid_t pid;
		int ret;
		if ((pid = fork()) == 0) {
			int null_fd = open("/dev/null", O_WRONLY);
			dup2(null_fd, 1);
			dup2(null_fd, 2);
			execl("/usr/bin/ffmpeg", "-nostdin", "-n", "-i", song_path_buffer, image_path_buffer, NULL);
		}
		waitpid(pid, &ret, 0);
	}*/
}

void destroy_album_list() {
	if (album_array == NULL) return;
	free(album_array);
	while (album_list != NULL) {
		album_type* last = album_list;
		album_list = last->next;
		free(last->filename);
		free(last->album_name);
		free(last->artist_name);
		free(last);
	}
}

void generate_song_list(album_type* current_album) {
	//Init song list
	song_list = NULL;
	song_count = 0;
	song_type* song_last = NULL;

	struct dirent *song_dp;
	char song_path_buffer[PATH_LENGTH_MAX];
	char song_name_buffer[NAME_LENGTH_MAX];
	DIR *song_dir;
	struct stat path_stat;

	if ((song_dir = opendir(current_album->filename)) == NULL) {
		fprintf(stderr, "open album directory failed\n");
		exit(1);
	}

	//Iterate over song directory
	while ((song_dp = readdir(song_dir)) != NULL) {
		strcpy(song_name_buffer, song_dp->d_name);
		if (strcmp(song_name_buffer, ".") == 0 || strcmp(song_name_buffer, "..") == 0) continue;

		//Generate song path
		strcpy(song_path_buffer, current_album->filename);
		strcat(song_path_buffer, "/");
		strcat(song_path_buffer, song_dp->d_name);

		//Song should be a regular file
		stat(song_path_buffer, &path_stat);
		if (!S_ISREG(path_stat.st_mode)) continue;

		//Create new song item
		song_type *current_song = malloc(sizeof(song_type));
		if (current_song == NULL) {
			fprintf(stderr, "Insufficient memory\n");
			exit(1);
		}
		current_song->next = NULL;
		current_song->id = song_count++;
		current_song->album_id = current_album->id;
		current_song->filename = malloc(strlen(song_path_buffer) + 1);
		current_song->song_name = malloc(strlen(song_name_buffer) + 1);
		if (current_song->filename == NULL || current_song->song_name == NULL) {
			fprintf(stderr, "Insufficient memory\n");
			exit(1);
		}
		strcpy(current_song->filename, song_path_buffer);
		strcpy(current_song->song_name, song_name_buffer);

		if (song_last == NULL) {
			song_last = song_list = current_song;
		} else {
			song_last->next = current_song;
			song_last = current_song;
		}
	}
	closedir(song_dir);

	//Generate array
	song_array = malloc(song_count * sizeof(song_type*));
	if (song_array == NULL) {
		fprintf(stderr, "Insufficient memory\n");
		exit(1);
	}
	song_type** song_iter = song_array;
	song_last = song_list;
	while (song_last != NULL) {
		*song_iter = song_last;
		song_iter++;
		song_last = song_last->next;
	}
}

void destroy_song_list() {
	if (song_array == NULL) return;
	free(song_array);
	while (song_list != NULL) {
		song_type* last = song_list;
		song_list = last->next;
		free(last->filename);
		free(last->song_name);
		free(last);
	}
}
