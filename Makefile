ifneq ($(GLX),)
CFLAGS=-DGLX -lX11
else
CFLAGS=`sdl-config --cflags` `sdl-config --libs`
endif
oo:oo.c
	gcc -std=gnu99 -Os -march=native -s -ffast-math -fwhole-program -o othello oo.c ${CFLAGS} -lGL