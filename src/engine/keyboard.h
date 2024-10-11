#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL2/SDL.h>

#include "game_state.h"
#include "player.h"
#include <stdio.h>

typedef struct _keyboard_map
{
    SDL_Scancode left;
    SDL_Scancode right;
    SDL_Scancode forward;
    SDL_Scancode backward;
    SDL_Scancode strafe_left;
    SDL_Scancode strafe_right;
    SDL_Scancode up;
    SDL_Scancode down;
    SDL_Scancode quit;
    SDL_Scancode toggleMap;
    SDL_Scancode debug_mode;
} keymap;

typedef struct _keyboard_states
{
    bool left;
    bool right;
    bool forward;
    bool backward;
    bool strafe_left;
    bool strafe_right;
    bool up;
    bool down;
    bool quit;
    bool toggleMap;
    bool debug_mode;
} keystates;

enum KEYBOARD_KEY_STATE
{
    KEY_STATE_UP,
    KEY_STATE_DOWN
};

void K_InitKeyMap();
void K_HandleEvents(game_state *game_state, player *player);
void K_ProcessKeyStates(player *player, double delta_time);
void K_HandleRealtimeKeys(SDL_Scancode key_scancode, enum KEYBOARD_KEY_STATE state);

#endif