#ifndef __RAILWAY_LIB_H__
#define __RAILWAY_LIB_H__

#include <gtk/gtk.h>

#define PATH_LENGTH_MAX 1000
#define NAME_LENGTH_MAX 1000

typedef struct album_type {
	int id;
	char *filename;
	char *album_name, *artist_name;
	struct album_type *next;
} album_type;

typedef struct song_type {
	int id;
	int album_id;
	char *filename;
	char *song_name;
	struct song_type *next;
	char *tag_title, *tag_album, *tag_artist;
	int tag_track_number, tag_disc_number;
} song_type;

extern size_t album_count, song_count;
extern album_type **album_array;
extern song_type **song_array;
extern char library_path[], album_cache_path[];

void init_library();
void destroy_library();
void generate_album_list();
void generate_song_list(const album_type*);
void destroy_album_list();
void destroy_song_list();
void generate_album_button_image(GTask*, void*, album_type*, void*);

#endif
