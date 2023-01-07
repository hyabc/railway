#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "railwaylib.h"

char *library_path, *album_cache_path;
album_type *album_list, **album_array;
song_type *song_list, **song_array;
size_t album_count, song_count;

GstDiscoverer *library_discoverer;

void init_library() {
	// Create configuration path
	char *config_path_buffer;
	if ((config_path_buffer = malloc(strlen(getenv("HOME")) + strlen("/.railway") + 1)) == NULL) {
		fprintf(stderr, "configuration error\n");
		exit(1);
	}
	strcpy(config_path_buffer, getenv("HOME"));
	strcat(config_path_buffer, "/.railway");

	//Read configuration LIBRARY_PATH using GTK+ library
	GKeyFile *keyfile = g_key_file_new();
	gchar *value;
	if (g_key_file_load_from_file(keyfile, config_path_buffer, G_KEY_FILE_NONE, NULL) == FALSE) {
		fprintf(stderr, "read configuration failed\n");
		exit(1);
	}

	//Get LIBRARY_PATH
	if ((value = g_key_file_get_string(keyfile, "railway", "LIBRARY_PATH", NULL)) == NULL || (library_path = malloc(strlen(value) + 1)) == NULL) {
		fprintf(stderr, "configuration error\n");
		exit(1);
	}
	strcpy(library_path, value);
	g_free(value);

	//Get ALBUM_CACHE_PATH
	if ((value = g_key_file_get_string(keyfile, "railway", "ALBUM_CACHE_PATH", NULL)) == NULL || (album_cache_path = malloc(strlen(value) + 1)) == NULL) {
		fprintf(stderr, "configuration error\n");
		exit(1);
	}
	strcpy(album_cache_path, value);
	g_free(value);

	//Free keyfile
	g_key_file_free(keyfile);
	free(config_path_buffer);

	//Init discoverer
	library_discoverer = gst_discoverer_new(1 * GST_SECOND, NULL);

	album_array = NULL;
	song_array = NULL;
}

void destroy_library() {
	g_object_unref(library_discoverer);
	free(library_path);
	free(album_cache_path);
}

void generate_album_list() {
	//Init album list
	album_list = NULL;
	album_count = 0;
	album_type* album_last = NULL;

	struct dirent *artist_dp, *album_dp;
	DIR *artist_dir, *album_dir;
	struct stat path_stat;

	if ((artist_dir = opendir(library_path)) == NULL) {
		fprintf(stderr, "open artist directory failed\n");
		exit(1);
	}

	//Iterate over artist directory
	while ((artist_dp = readdir(artist_dir)) != NULL) {
		if (strcmp(artist_dp->d_name, ".") == 0 || strcmp(artist_dp->d_name, "..") == 0) continue;

		char *artist_path_buffer, *artist_name_buffer;
		if ((artist_name_buffer = malloc(strlen(artist_dp->d_name) + 1)) == NULL || (artist_path_buffer = malloc(strlen(library_path) + 1 + strlen(artist_dp->d_name) + 1)) == NULL) {
			fprintf(stderr, "Insufficient memory\n");
			exit(1);
		}
		strcpy(artist_name_buffer, artist_dp->d_name);

		//Generate artist path
		strcpy(artist_path_buffer, library_path);
		strcat(artist_path_buffer, "/");
		strcat(artist_path_buffer, artist_dp->d_name);

		//Iterate over album directory
		if ((album_dir = opendir(artist_path_buffer)) == NULL) {free(artist_path_buffer);free(artist_name_buffer);continue;}
		while ((album_dp = readdir(album_dir)) != NULL) {
			if (strcmp(album_dp->d_name, ".") == 0 || strcmp(album_dp->d_name, "..") == 0) continue;

			char *album_path_buffer, *album_name_buffer;
			if ((album_name_buffer = malloc(strlen(album_dp->d_name) + 1)) == NULL || (album_path_buffer = malloc(strlen(artist_path_buffer) + 1 + strlen(album_dp->d_name) + 1)) == NULL) {
				fprintf(stderr, "Insufficient memory\n");
				exit(1);
			}
			strcpy(album_name_buffer, album_dp->d_name);

			//Generate album path
			strcpy(album_path_buffer, artist_path_buffer);
			strcat(album_path_buffer, "/");
			strcat(album_path_buffer, album_dp->d_name);

			//This should be a directory if it is an album folder
			stat(album_path_buffer, &path_stat);
			if (S_ISDIR(path_stat.st_mode)) {
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
			free(album_path_buffer);
			free(album_name_buffer);
		}
		closedir(album_dir);
		free(artist_path_buffer);
		free(artist_name_buffer);
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
}

void generate_album_button_image(GTask* gtask, void* x, album_type *current_album, void* y) {
	struct stat path_stat;

	//Generate image path to save
	char *image_path_buffer;
	if ((image_path_buffer = malloc(strlen(album_cache_path) + 1 + strlen(current_album->artist_name) + 1 + strlen(current_album->album_name) + strlen(".jpg") + 1)) == NULL) {
		fprintf(stderr, "Insufficient memory\n");
		exit(1);
	}
	strcpy(image_path_buffer, album_cache_path);
	strcat(image_path_buffer, "/");
	strcat(image_path_buffer, current_album->artist_name);
	strcat(image_path_buffer, "-");
	strcat(image_path_buffer, current_album->album_name);
	strcat(image_path_buffer, ".jpg");

	//Try to find one file as song
	struct dirent *song_dp;
	DIR *song_dir;
	char *song_path_buffer;
	bool exist_any_song = false;
	if ((song_dir = opendir(current_album->filename)) == NULL) {
		return;
	}
	while ((song_dp = readdir(song_dir)) != NULL) {
		if (strcmp(song_dp->d_name, ".") == 0 || strcmp(song_dp->d_name, "..") == 0) continue;

		if ((song_path_buffer = malloc(strlen(current_album->filename) + 1 + strlen(song_dp->d_name) + 1)) == NULL) {
			fprintf(stderr, "Insufficient memory\n");
			exit(1);
		}
		strcpy(song_path_buffer, current_album->filename);
		strcat(song_path_buffer, "/");
		strcat(song_path_buffer, song_dp->d_name);

		stat(song_path_buffer, &path_stat);
		if (S_ISREG(path_stat.st_mode)) {
			exist_any_song = true;
			break;
		}
	}
	closedir(song_dir);

	if (!exist_any_song) return;

	if (stat(image_path_buffer, &path_stat) != 0) {
		//Execute ffmpeg to save album art image
		pid_t pid;
		int ret;
		if ((pid = fork()) == 0) {
			int null_fd = open("/dev/null", O_WRONLY);
			dup2(null_fd, STDOUT_FILENO);
			dup2(null_fd, STDERR_FILENO);
			execl("/usr/bin/ffmpeg", "ffmpeg", "-nostdin", "-n", "-i", song_path_buffer, image_path_buffer, NULL);
		}
		waitpid(pid, &ret, 0);
	}

	free(song_path_buffer);
	free(image_path_buffer);
}

void destroy_album_list() {
	if (album_array == NULL) return;
	free(album_array);
	while (album_list != NULL) {
		album_type* last = album_list;
		album_list = last->next;

		//Free strings
		free(last->filename);
		free(last->album_name);
		free(last->artist_name);

		//Free this node
		free(last);
	}
	album_array = NULL;
}

void generate_song_tags() {
	for (int i = 0;i < song_count;i++) {
		//Init tag fields in song_type in case not available
		song_array[i]->tag_track_number = 0;
		song_array[i]->tag_disc_number = 0;
		song_array[i]->tag_title = song_array[i]->tag_album = song_array[i]->tag_artist = NULL;

		//Create song uri
		char *song_path_buffer;
		if ((song_path_buffer = malloc(strlen("file://") + strlen(song_array[i]->filename) + 1)) == NULL) {
			fprintf(stderr, "Insufficient memory\n");
			exit(1);
		}
		strcpy(song_path_buffer, "file://");
		strcat(song_path_buffer, song_array[i]->filename);

		//Call discover
		GstDiscovererInfo *info = gst_discoverer_discover_uri(library_discoverer, song_path_buffer, NULL);
		if (info == NULL) {free(song_path_buffer);continue;}
		const GstTagList *tags = gst_discoverer_info_get_tags(info);
		if (tags == NULL) {free(song_path_buffer);continue;}

		//Get song duration
		song_array[i]->duration = GST_TIME_AS_SECONDS(gst_discoverer_info_get_duration(info));

		//Get these fields
		char* str;
		if (gst_tag_list_get_string(tags, "title", &str)) {
			song_array[i]->tag_title = malloc(strlen(str) + 1);
			strcpy(song_array[i]->tag_title, str);
			g_free(str);
		}
		if (gst_tag_list_get_string(tags, "album", &str)) {
			song_array[i]->tag_album = malloc(strlen(str) + 1);
			strcpy(song_array[i]->tag_album, str);
			g_free(str);
		}
		if (gst_tag_list_get_string(tags, "artist", &str)) {
			song_array[i]->tag_artist = malloc(strlen(str) + 1);
			strcpy(song_array[i]->tag_artist, str);
			g_free(str);
		}
		unsigned int track_number, disc_number;
		if (gst_tag_list_get_uint(tags, "track-number", &track_number)) {
			song_array[i]->tag_track_number = track_number;
		}
		if (gst_tag_list_get_uint(tags, "album-disc-number", &disc_number)) {
			song_array[i]->tag_disc_number = disc_number;
		}

		//Delete info
		g_object_unref(info);
		free(song_path_buffer);
	}
}

int song_track_number_compare(const void *x, const void *y) {
	if (((*(song_type**)(x)))->tag_disc_number != ((*(song_type**)(y)))->tag_disc_number)
		return ((*(song_type**)(x)))->tag_disc_number < ((*(song_type**)(y)))->tag_disc_number ? -1 : 1;
	else
		return ((*(song_type**)(x)))->tag_track_number < (*(song_type**)(y))->tag_track_number ? -1 : 1;
}

void generate_song_list(const album_type* current_album) {
	//Init song list
	song_list = NULL;
	song_count = 0;
	song_type* song_last = NULL;

	struct dirent *song_dp;
	DIR *song_dir;
	struct stat path_stat;

	if ((song_dir = opendir(current_album->filename)) == NULL) {
		fprintf(stderr, "open album directory failed\n");
		exit(1);
	}

	//Iterate over song directory
	while ((song_dp = readdir(song_dir)) != NULL) {
		if (strcmp(song_dp->d_name, ".") == 0 || strcmp(song_dp->d_name, "..") == 0) continue;

		char *song_path_buffer, *song_name_buffer;
		if ((song_path_buffer = malloc(strlen(current_album->filename) + 1 + strlen(song_dp->d_name) + 1)) == NULL || (song_name_buffer = malloc(strlen(song_dp->d_name) + 1)) == NULL) {
			fprintf(stderr, "Insufficient memory\n");
			exit(1);
		}
		//Generate song path
		strcpy(song_name_buffer, song_dp->d_name);

		//Generate song path
		strcpy(song_path_buffer, current_album->filename);
		strcat(song_path_buffer, "/");
		strcat(song_path_buffer, song_dp->d_name);

		//Song should be a regular file
		stat(song_path_buffer, &path_stat);
		if (S_ISREG(path_stat.st_mode)) {
			//Create new song item
			song_type *current_song = malloc(sizeof(song_type));
			if (current_song == NULL) {
				fprintf(stderr, "Insufficient memory\n");
				exit(1);
			}
			current_song->next = NULL;
			song_count++;
			current_song->album_id = current_album->id;
			current_song->filename = malloc(strlen(song_path_buffer) + 1);
			current_song->song_name = malloc(strlen(song_name_buffer) + 1);
			if (current_song->filename == NULL || current_song->song_name == NULL) {
				fprintf(stderr, "Insufficient memory\n");
				exit(1);
			}
			strcpy(current_song->filename, song_path_buffer);
			strcpy(current_song->song_name, song_name_buffer);

			//Insert to song list
			if (song_last == NULL) {
				song_last = song_list = current_song;
			} else {
				song_last->next = current_song;
				song_last = current_song;
			}
		}

		free(song_path_buffer);
		free(song_name_buffer);
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

	//Generate song tags
	generate_song_tags();

	//Sort with tag_track_number
	qsort(song_array, song_count, sizeof(song_type*), song_track_number_compare);

	//Set id
	for (int i = 0;i < song_count;i++) {
		song_array[i]->id = i;
	}
}

void destroy_song_list() {
	if (song_array == NULL) return;
	free(song_array);
	while (song_list != NULL) {
		song_type* last = song_list;
		song_list = last->next;

		//Free strings in last
		free(last->filename);
		free(last->song_name);
		if (last->tag_title != NULL) free(last->tag_title);
		if (last->tag_album != NULL) free(last->tag_album);
		if (last->tag_artist != NULL) free(last->tag_artist);

		//Free this node
		free(last);
	}
	song_array = NULL;
}
