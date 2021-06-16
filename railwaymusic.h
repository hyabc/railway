#include <stdbool.h>

#ifndef __RAILWAY_MUSIC_H__
#define __RAILWAY_MUSIC_H__

void music_play(const char*);
void music_volume(double);
bool music_pause_trigger();
void init_music();
void destroy_music();

#endif
