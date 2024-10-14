all:
	gcc -Iinclude -Llib -o bin/main src/main.c src/engine/*.c -lSDL2main -lSDL2

test:
	gcc -o test test.c

final:
	gcc -Iinclude -Llib -o bin/main src/main.c src/engine/*.c -lSDL2main -lSDL2 -mwindows