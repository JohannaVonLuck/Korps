# Use the GNU C/C++ compiler:
C = gcc
CC = g++

# Turn on optimization and warnings:
CFLAGS = -Wall -pedantic -O3
SDL_FLAGS = -D_REENTRANT

# How to compile:
.c.o:
	$(C) $(CFLAGS) $(SDL_FLAGS) -c $<

.cc.o:
	$(CC) $(CFLAGS) $(SDL_FLAGS) -c $<

.cpp.o:
	$(CC) $(CFLAGS) $(SDL_FLAGS) -c $<

# How to link:
LINK = $(CC)

# OpenGL/Mesa libraries for Linux
GL_LIBS = -lGLU -lGL -l3ds
SDL_LIBS = -lSDL -lpthread -lSDL_image -lSDL_ttf
AL_LIBS = -lopenal -lvorbisfile

#-----------------------------------------------------------------------
# Specific targets:

# MAKE allows the use of "wildcards", to make writing compilation instructions
# a bit easier. GNU make uses $@ for the target and $^ for the dependencies.

all:	main

main:	atg.o camera.o collision.o console.o database.o effects.o fonts.o gameloop.o load.o object.o objhandler.o objlist.o objmodules.o objunit.o metrics.o misc.o model.o projectile.o scenery.o script.o sounds.o tank.o texture.o ui.o main.o
	$(LINK) -o "../main" $^ $(GL_LIBS) $(SDL_LIBS) $(AL_LIBS)

clean:
	rm -f *.o
