#include <gst/gst.h>
#include <gst/player/player.h>
#include "railwaylib.h"

#ifndef __RAILWAY_H__
#define __RAILWAY_H__

void music_position_update_cb(GstPlayer*, GstClockTime, void*);
void play_song(song_type*);
void update_songs_cb(GtkWidget*, album_type*);

#endif
