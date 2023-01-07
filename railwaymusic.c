#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdatomic.h>
#include <sys/wait.h>
#include "railway.h"
#include "railwaylib.h"
#include "railwayplaylist.h"
#include "railwaymusic.h"

bool music_play_state;
int music_duration;
atomic_int music_atomic_is_stopped, music_atomic_position;
FILE *music_player_out, *music_player_in;
pid_t music_player_pid;
pthread_t music_wait_thread;

void* music_wait(void* x) {
	char buffer[256], op;
	while (true) {
		pthread_testcancel();
		fgets(buffer, 256, music_player_in);
		sscanf(buffer, "@%c", &op);
		if (op == 'P') {
			int state;
			sscanf(buffer, "@P %d", &state);
			music_atomic_is_stopped = state == 0 ? 1 : 0;
		} else if (op == 'F') {
			int fc, fl;
			float sec, secl;
			sscanf(buffer, "@F %d %d %f %f", &fc, &fl, &sec, &secl);
			music_atomic_position = (int)(sec);
		}
	}
}

int music_update(void* x) {
	if (music_play_state && music_atomic_is_stopped) {
		playlist_next();
	}
	music_position_update_cb(music_atomic_position, music_duration);
	return true;
}

void music_play(song_type *current_song) {
	//Stop current player
	if (music_player_out != NULL) {
		fprintf(music_player_out, "stop\n");
		fflush(music_player_out);
	}

	//Play song file
	fprintf(music_player_out, "load %s\n", current_song->filename);
	fflush(music_player_out);

	//Set playing state
	music_play_state = true;
	music_duration = current_song->duration;
}

void music_pause_trigger() {
	music_play_state = !music_play_state;
	fprintf(music_player_out, "pause\n");
	fflush(music_player_out);
}

bool music_is_playing() {
	return music_play_state;
}

void music_volume(double value) {
	fprintf(music_player_out, "volume %d\n", (int)(value * 100.0));
	fflush(music_player_out);
}

void init_music() {
	//Init variables
	music_play_state = false;
	music_duration = 0;
	music_atomic_position = 0;
	music_atomic_is_stopped = true;

	//Create pipes
	int player_in[2], player_out[2];
	if (pipe(player_in) == -1 || pipe(player_out) == -1) {
		perror("Pipe error");
		exit(1);
	}

	//Launch mpg123
	if ((music_player_pid = fork()) == 0) {
		close(player_out[1]);
		close(player_in[0]);
		dup2(player_out[0], STDIN_FILENO);
		dup2(player_in[1], STDOUT_FILENO);
		execl("/usr/bin/mpg123", "mpg123", "-R", NULL);
	}
	close(player_out[0]);
	close(player_in[1]);
	music_player_out = fdopen(player_out[1], "w");
	music_player_in = fdopen(player_in[0], "r");

	//Launch thread and set update function
	pthread_create(&music_wait_thread, NULL, music_wait, NULL);
	g_timeout_add(1000, music_update, NULL);
}

void destroy_music() {
	pthread_cancel(music_wait_thread);
	pthread_join(music_wait_thread, NULL);
	fprintf(music_player_out, "stop\nquit\n");
	fflush(music_player_out);
	waitpid(music_player_pid, NULL, 0);
}
