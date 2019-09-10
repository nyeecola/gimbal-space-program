all: build run

build: src/*.c
	mkdir -p build
	gcc -Wall -g src/main.c -o build/main.x -Iinclude -lglfw -lGLEW -lGL -lm -lGLU

run:
	build/main.x

clean:
	rm -r build

