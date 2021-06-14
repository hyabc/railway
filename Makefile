all: railway railwaylib

railway.o: railway.c
	gcc railway.c -c -o railway.o `pkg-config --cflags gtk+-3.0` -Wall

railway: railway.o
	gcc railway.o -o railway `pkg-config --libs gtk+-3.0` -Wall

railwaylib.o: railwaylib.c
	gcc railwaylib.c -c -o railwaylib.o `pkg-config --cflags gtk+-3.0` -Wall

railwaylib: railwaylib.o
	gcc railwaylib.o -o railwaylib `pkg-config --libs gtk+-3.0` -Wall

clean:
	rm railway railwaylib *.o
