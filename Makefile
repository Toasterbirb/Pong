CC=g++
outputDir=./build
WarningFlags=-Wpedantic -pedantic -Wall -Wextra
SDL_FLAGS=-lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx

all: pong

pong: pong.o
	mkdir -p build
	rsync -av ./res ./build/
	$(CC) $^ $(SDL_FLAGS) -lbirb2d $(WarningFlags) -o $(outputDir)/pong

pong.o: ./src/pong.cpp
	$(CC) -O2 -c $^ -o pong.o

clean:
	rm -rf build
	rm -f *.o
