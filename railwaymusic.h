#include <stdbool.h>
#include <gtk/gtk.h>

#ifndef __RAILWAY_MUSIC_H__
#define __RAILWAY_MUSIC_H__

void music_play(const char*);
void music_volume(double);
void music_pause_trigger();
GstClockTime music_duration();
GstClockTime music_position();
void music_seek(GstClockTime);
void init_music();
void destroy_music();

#endif
