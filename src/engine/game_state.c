#include "game_state.h"

int frameStart = 0;

game_state G_Init(const unsigned int screenW, const unsigned int screenH, int target_FPS)
{
    game_state game;
    game.isRunning = true;
    game.isDebugMode = false;
    game.screen_w = screenW;
    game.screen_h = screenH;
    game.target_fps = target_FPS;
    game.target_frameTime = 1.0 / (double) game.target_fps;
    game.isFpsCapped = false;
    game.deltatime = game.target_frameTime;
    
    return game;
}

void G_FrameStart()
{
    frameStart = SDL_GetTicks();
}
void G_FrameEnd(game_state *state)
{
    state->deltatime = (SDL_GetTicks() - frameStart) / 1000.0;

    if (state->deltatime < state->target_frameTime)
    {
        SDL_Delay((state->target_frameTime - state->deltatime) * 1000.0);
        state->deltatime = state->target_frameTime;
    }
}