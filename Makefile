all:
	gcc -Iinclude -Llib -o bin/main src/main.c src/engine/*.c -lSDL2main -lSDL2