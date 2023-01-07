CFLAGS=-Wall -O2 `pkg-config --cflags gtk+-3.0 gstreamer-1.0`
LDFLAGS=`pkg-config --libs gtk+-3.0 gstreamer-1.0 gstreamer-pbutils-1.0` -pthread

all: railway

%.o: %.c
	gcc $^ -c -o $@ $(CFLAGS)

railway: railway.o railwaylib.o railwaymusic.o railwayplaylist.o
	gcc $^ -o $@ $(LDFLAGS)

clean:
	rm railway *.o

install: railway
	cp railway.desktop ~/.local/share/applications/
	mkdir -p ~/.local/bin/railway
	cp railway railway.ui ~/.local/bin/railway/

uninstall:
	rm -rf ~/.local/share/applications/railway.desktop
	rm -rf ~/.local/bin/railway
