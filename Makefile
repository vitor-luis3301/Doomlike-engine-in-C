all:
	gcc -Iinclude -Llib -o bin/main src/main.c src/engine/*.c src/editor/*.c -lSDL2main -lSDL2