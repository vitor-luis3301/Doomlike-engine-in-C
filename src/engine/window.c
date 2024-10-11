#include "window.h"

SDL_Window *sdl_win = NULL;

void W_Init(const unsigned int windowW, const unsigned int windowH)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    sdl_win = SDL_CreateWindow(
        "Engine",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        windowW, windowH,
        SDL_WINDOW_SHOWN
    );
}
void W_Shutdown()
{
    SDL_DestroyWindow(sdl_win);
    SDL_Quit();
}
SDL_Window* W_Get()
{
    return sdl_win;
}