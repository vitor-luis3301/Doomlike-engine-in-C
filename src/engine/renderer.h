#ifndef RENDERER_H
#define RENDERER_H

#define SDL_MAIN_HANDLED

#include <stdio.h>
#include "typedefs.h"
#include "player.h"
#include "game_state.h"
#include "utils.h"

typedef struct _r_plane
{
    int t[1024];
    int b[1024];
} plane_lookup;

typedef struct _wall
{
    vector2 a;
    vector2 b;
    double portal_top_h;
    double portal_bottom_h;
    bool is_portal;
} wall;

typedef struct _sector 
{
    int id;
    wall walls[10];
    int num_walls;
    int height;
    int elevation;
    double dist;
    unsigned int color;
    unsigned int floor_color;
    unsigned int ceiling_color;

    plane_lookup portal_floorx_y;
    plane_lookup portal_ceilingx_y;
    plane_lookup floorx_y;
    plane_lookup ceilingx_y;
} sector;

typedef struct _sectors_queue
{
    sector sectors[1024];
    int num_sectors;
} sectors_queue;

void R_Init(SDL_Window *window, game_state *state);
void R_Shutdown();
void R_Render(player *player, game_state *state, double fov);
void R_Draw_Walls(player *player, game_state *state);
sector R_CreateSector(int height, int elevation, unsigned int color, unsigned int ceiling_color, unsigned int floor_color);
void R_SectorAddWall(sector *sector, wall vertices);
void R_AddSectorToQueue(sector *sector);
wall R_CreateWall(int ax, int ay, int bx, int by);
wall R_CreatePortal(int ax, int ay, int bx, int by, int th, int bh);

#endif