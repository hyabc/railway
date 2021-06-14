all: railway

railway: railway.c
	gcc railway.c -o railway `pkg-config --cflags --libs gtk+-3.0` -Wall

clean:
	rm railway
