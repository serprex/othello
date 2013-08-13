ifneq ($(GLX),)
CFLAGS=-DGLX -lX11
else
CFLAGS=`sdl2-config --cflags` `sdl2-config --libs`
endif
oo:oo.c
	gcc -std=gnu99 -Os -march=native -s -ffast-math -fwhole-program -o othello oo.c ${CFLAGS} -lGL