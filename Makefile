all: railway

railway.o: railway.c
	gcc railway.c -c -o railway.o `pkg-config --cflags gtk+-3.0 gstreamer-1.0` -Wall

railwayplaylist.o: railwayplaylist.c
	gcc railwayplaylist.c -c -o railwayplaylist.o `pkg-config --cflags gtk+-3.0 gstreamer-1.0`  -Wall

railwaylib.o: railwaylib.c
	gcc railwaylib.c -c -o railwaylib.o `pkg-config --cflags gtk+-3.0 gstreamer-1.0 gstreamer-player-1.0 gstreamer-pbutils-1.0` -Wall

railwaymusic.o: railwaymusic.c
	gcc railwaymusic.c -c -o railwaymusic.o `pkg-config --cflags gtk+-3.0 gstreamer-1.0 gstreamer-player-1.0` -Wall

railway: railway.o railwaylib.o railwaymusic.o railwayplaylist.o
	gcc railway.o railwaylib.o railwaymusic.o railwayplaylist.o -o railway `pkg-config --libs gtk+-3.0 gstreamer-1.0 gstreamer-player-1.0 gstreamer-pbutils-1.0` -pthread -Wall

clean:
	rm railway *.o

install: railway
	cp railway.desktop ~/.local/share/applications/
	mkdir -p ~/.local/bin/railway
	cp railway railway.ui ~/.local/bin/railway/

uninstall:
	rm -rf ~/.local/share/applications/railway.desktop
	rm -rf ~/.local/bin/railway
