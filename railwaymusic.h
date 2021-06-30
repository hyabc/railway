#include <stdbool.h>
#include <gtk/gtk.h>

#ifndef __RAILWAY_MUSIC_H__
#define __RAILWAY_MUSIC_H__

void music_play(song_type*);
void music_volume(double);
void music_pause_trigger();
bool music_is_playing();
void init_music();
void destroy_music();

#endif
