typedef struct album_type {
	int id;
	char *filename;
	char *album_name, *artist_name;
};

typedef struct song_type {
	int id;
	char *filename;
	char *album_name, *artist_name, *song_name;
};
