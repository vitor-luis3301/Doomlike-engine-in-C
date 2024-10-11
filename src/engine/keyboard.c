#include "keyboard.h"

keymap keyMap;
keystates keyState;

const double moveSpeed = 150.0;
const double elevationSpeed = 500.0 * 100;
const double rotationSpeed = 4;

void K_InitKeyMap()
{
    keyMap.forward = SDL_SCANCODE_W;
    keyMap.backward = SDL_SCANCODE_S;
    keyMap.left = SDL_SCANCODE_LEFT;
    keyMap.right = SDL_SCANCODE_RIGHT;
    keyMap.strafe_left = SDL_SCANCODE_A;
    keyMap.strafe_right = SDL_SCANCODE_D;
    keyMap.up = SDL_SCANCODE_SPACE;
    keyMap.down = SDL_SCANCODE_LCTRL;
    keyMap.quit = SDL_SCANCODE_ESCAPE;
    keyMap.toggleMap = SDL_SCANCODE_M;
    keyMap.debug_mode = SDL_SCANCODE_O;

    keyState.forward = false;
    keyState.backward = false;
    keyState.left = false;
    keyState.right = false;
    keyState.strafe_left = false;
    keyState.strafe_right = false;
    keyState.up = false;
    keyState.down = false;
    keyState.quit = false;
    keyState.toggleMap = false;
    keyState.debug_mode = false;
}

void K_HandleRealTimeKeys(SDL_Scancode key_scancode, enum KEYBOARD_KEY_STATE state)
{
    if (key_scancode == keyMap.forward)
        keyState.forward = state;
    else if (key_scancode == keyMap.backward)
        keyState.backward = state;

    if (key_scancode == keyMap.left)
        keyState.left = state;
    else if (key_scancode == keyMap.right)
        keyState.right = state;
    

    if (key_scancode == keyMap.strafe_left)
        keyState.strafe_left = state;
    else if (key_scancode == keyMap.strafe_right)
        keyState.strafe_right = state;
    
    if (key_scancode == keyMap.up)
        keyState.up = state;
    else if (key_scancode == keyMap.down)
        keyState.down = state;
    
    if (key_scancode == keyMap.toggleMap && state == true)
        keyState.toggleMap = !keyState.toggleMap;
}

void K_ProcessedKeyStates(player *p, double delta_time)
{
    if (keyState.forward)
    {
        p->position.x += moveSpeed * cos(p->direction_angle) * delta_time;
        p->position.y += moveSpeed * sin(p->direction_angle) * delta_time;
    }
    else if (keyState.backward)
    {
        p->position.x -= moveSpeed * cos(p->direction_angle) * delta_time;
        p->position.y -= moveSpeed * sin(p->direction_angle) * delta_time;
    }

    if (keyState.left)
        p->direction_angle += rotationSpeed * delta_time;
    else if (keyState.right)
        p->direction_angle -= rotationSpeed * delta_time;
    
    if (keyState.strafe_left)
    {
        p->position.x += moveSpeed * cos(p->direction_angle + M_PI / 2) * delta_time;
        p->position.y += moveSpeed * sin(p->direction_angle + M_PI / 2) * delta_time;
    }
    else if (keyState.strafe_right)
    {
        p->position.x -= moveSpeed * cos(p->direction_angle + M_PI / 2) * delta_time;
        p->position.y -= moveSpeed * sin(p->direction_angle + M_PI / 2) * delta_time;
    }

    if (keyState.up)
        p->z += elevationSpeed * delta_time;
    else if (keyState.down)
        p->z -= elevationSpeed * delta_time;

}

void K_HandleEvents(game_state *state, player *player)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type)
    {
    case SDL_KEYDOWN:
        K_HandleRealTimeKeys(event.key.keysym.scancode, KEY_STATE_DOWN);
        state->state_showMap = keyState.toggleMap;

        if (event.key.keysym.scancode == keyMap.quit)
            state->isRunning = false;
        if (event.key.keysym.scancode == keyMap.debug_mode)
            state->isDebugMode = !state->isDebugMode;
        break;
    
    case SDL_KEYUP:
        K_HandleRealTimeKeys(event.key.keysym.scancode, KEY_STATE_UP);
        break;
    
    case SDL_QUIT:
        state->isRunning = false;
    default:
        break;
    }

    K_ProcessedKeyStates(player, state->deltatime);
}