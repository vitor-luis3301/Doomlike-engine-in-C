#ifndef GAME_STATE_H
#define GAME_STATE_H

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>

#include "typedefs.h"

typedef struct _game_state
{
    unsigned int screen_w;
    unsigned int screen_h;
    double target_fps;
    double target_frameTime;
    double deltatime;
    bool isRunning;
    bool isPaused;
    bool isFpsCapped;
    bool state_showMap;
    bool isDebugMode;
} game_state;

game_state G_Init(const unsigned int screenW, const unsigned int screenH, int target_FPS);
void G_FrameStart();
void G_FrameEnd(game_state *state);

#endif /* G_GAME_STATE_H */