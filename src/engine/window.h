#ifndef WINDOW_H
#define WINDOW_H

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>

void W_Init(const unsigned int winw, const unsigned int winh);
void W_Shutdown();
SDL_Window* W_Get();

#endif