#include <gtk/gtk.h>

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
} song_type;

extern size_t album_count, song_count;
extern album_type **album_array;
extern song_type **song_array;

void init_library();
void generate_album_list();
void generate_song_list(album_type*);
void destroy_album_list();
void destroy_song_list();
void generate_album_button_image(GtkWidget**);
