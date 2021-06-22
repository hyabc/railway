#include "railwaylib.h"
#include <stdbool.h>

#ifndef __RAILWAY_PLAYLIST_H__
#define __RAILWAY_PLAYLIST_H__

void playlist_play(song_type*);
void init_playlist();
void destroy_playlist();
void playlist_next();
void playlist_prev();
void playlist_set_shuffle(bool);

#endif
