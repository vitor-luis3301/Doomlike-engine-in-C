#include "renderer.h"

#define IS_WALL 0
#define IS_CEILING 1
#define IS_FLOOR 2

#define CEILING_COLOR 0x3ac960
#define FLOOR_COLOR 0x1a572a

SDL_Window* window;
SDL_Renderer* sdl_renderer;
SDL_Texture* screen_texture;
unsigned int scrnw, scrnh;

bool isDebugMode = false;
unsigned int *screen_buffer = NULL;
int screen_buffer_size = 0;

sectors_queue queue;

typedef struct _rquad
{
    int ax, bx; // X coordinates of points A & B
    int at, ab; // A top & B bottom coordinates
    int bt, bb; // B top & B bottom coordinates
} rquad;

void R_ShutdownScreen()
{
    if (screen_texture)
        SDL_DestroyTexture(screen_texture);

    if (screen_buffer != NULL)
        free(screen_buffer);
}

void R_Shutdown()
{
    R_ShutdownScreen();
    SDL_DestroyRenderer(sdl_renderer);
}

void R_UpdateScreen()
{
    SDL_UpdateTexture(screen_texture, NULL, screen_buffer, scrnw * sizeof(unsigned int));
    SDL_RenderCopy(sdl_renderer, screen_texture, NULL, NULL);
    SDL_RenderPresent(sdl_renderer);
}

void R_InitScreen(int w, int h)
{
    screen_buffer_size = sizeof(unsigned int) * w * h;
    screen_buffer = (unsigned int*)malloc(screen_buffer_size);
    if (screen_buffer == NULL)
    {
        screen_buffer_size = -1;
        printf("Error initializing screen buffer!\n");
        R_Shutdown();
    }

    memset(screen_buffer, 0, screen_buffer_size);

    screen_texture = SDL_CreateTexture(
        sdl_renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        w, h
    );

    if (screen_texture == NULL)
    {
        printf("Error creating screen texture!\n");
        R_Shutdown();
    }
}

void R_Init(SDL_Window *main_win, game_state *state)
{
    window = main_win;
    scrnw = state->screen_w / 2;
    scrnh = state->screen_h / 2;

    sdl_renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
    R_InitScreen(scrnw, scrnh);
    SDL_RenderSetLogicalSize(sdl_renderer, scrnw, scrnh);
}

void R_DrawPoint(int x, int y, unsigned int color)
{
    bool is_out_of_bounds = (x < 0 || x > scrnw || y < 0 || y >= scrnh);
    bool is_outside_mem_buff = (scrnw * y + x) >= (scrnw * scrnh);

    if (is_out_of_bounds || is_outside_mem_buff)
        return;

    screen_buffer[scrnw * y + x] = color;
}

void R_DrawLine(int x0, int y0, int x1, int y1, unsigned int color)
{
    int dx;
    if (x1 > x0)
        dx = x1 - x0;
    else
        dx = x0 - x1;

    int dy;
    if (y1 > y0)
        dy = y1 - y0;
    else
        dy = y0 - y1;

    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;)
    {
        R_DrawPoint(x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;

        e2 = err;

        if (e2 > -dx)
        {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dy)
        {
            err += dx;
            y0 += sy;
        }
    }

    if (isDebugMode)
    {
        R_UpdateScreen();
        SDL_Delay(10);
    }
}

void R_ClearScreenBuffer()
{
    memset(screen_buffer, 0, sizeof(uint32_t) * scrnw * scrnh);
}

void R_SwapQuadPoints(rquad *q)
{
    int t = q->bx;
    q->bx = q->ax;
    q->ax = t;

    t = q->bt;
    q->bt = q->at;
    q->at = t;

    t = q->bb;
    q->bb = q->ab;
    q->ab = t;
}

void R_CalcInterpolationFactors(rquad q, double *delta_height, double *delta_elevation)
{
    // absolute width
    int width = abs(q.ax - q.bx);
    if (width == 0)
    {
        *delta_height = -1;
        *delta_elevation = -1;
        return;
    }

    // calc height increment
    int a_height = q.ab - q.at;
    int b_height = q.bb - q.bt;

    *delta_height = (double)(b_height - a_height) / (double)width;

    // get Player's view elevation from the floor
    int y_center_a = (q.ab - (a_height / 2));
    int y_center_b = (q.bb - (b_height / 2));

    *delta_elevation = (y_center_b - y_center_a) / (double)width;
}

int R_CapToScreenH(int val)
{
    if (val < 0) val = 0;
    if (val > scrnh) val = scrnh;

    return val;
}

int R_CapToScreenW(int val)
{
    if (val < 0) val = 0;
    if (val > scrnw) val = scrnw - 1;

    return val;
}

void R_Rasterize(rquad q, uint32_t color, int ceil_floor_wall, plane_lookup *xy_lut)
{
    // if backfacing wall then do not rasterize
    if (ceil_floor_wall == IS_WALL && q.ax > q.bx)
        return;

    bool is_back_wall = false;

    if ((ceil_floor_wall != IS_WALL) && q.ax > q.bx)
    {
        R_SwapQuadPoints(&q);
        is_back_wall = true;
    }

    double delta_height, delta_elevation;

    R_CalcInterpolationFactors(q, &delta_height, &delta_elevation);
    if (delta_height == -1 && delta_elevation == -1)
        return;

    for (int x = q.ax, i = 1; x < q.bx; x++, i++)
    {
        if (x < 0 || x > scrnw-1) continue;

        double dh = delta_height * i;
        double dy_player_elev = delta_elevation * i;

        int y1 = q.at - (dh / 2) + dy_player_elev;
        int y2 = q.ab + (dh / 2) + dy_player_elev;

        y1 = R_CapToScreenH(y1);
        y2 = R_CapToScreenH(y2);

        if (ceil_floor_wall == IS_CEILING)
        {
            // save the ceiling Y coordinates for each X coordinate
            if (!is_back_wall)
                xy_lut->t[x] = y1;
            else
                xy_lut->b[x] = y1;
        }
        else if (ceil_floor_wall == IS_FLOOR)
        {
            // save the floor's Y coordinates for each X coordinate
            if (!is_back_wall)
                xy_lut->t[x] = y2;
            else
                xy_lut->b[x] = y2;
        }
        else 
        {
            // rasterize
            R_DrawLine(x, y1, x, y2, color);
        }
    }
}

rquad R_CreateRendarableQuad(int ax, int bx, int at, int ab, int bt, int bb)
{
    rquad quad =
    {
        .ax = ax, .bx = bx,
        .at = at, .ab = ab,
        .bt = bt, .bb = bb
    };

    return quad;
}

void R_ClipBehindPlayer(double *ax, double *ay, double bx, double by)
{
    double px1 = 1;
    double py1 = 1;
    double px2 = 200;
    double py2 = 1;

    double a = (px1 - px2) * (*ay - py2) - (py1 - py2) * (*ax - px2);
    double b = (py1 - py2) * (*ax - bx) - (px1 - px2) * (*ay - by);

    double t = a / b;

    *ax = *ax - (t * (bx - *ax));
    *ay = *ay - (t * (by - *ay));
}

vector2 R_CalcCentroid(sector *s)
{
    vector2 centroid = {0};
    for (int i = 0; i < s->num_walls; i++)
    {
        centroid.x += s->walls[i].a.x + s->walls[i].b.x;
        centroid.y += s->walls[i].a.y + s->walls[i].b.y;
    }

    centroid.x /= s->num_walls * 2;
    centroid.y /= s->num_walls * 2;

    return centroid;
}

double R_DistanceToPoint(vector2 a, vector2 b)
{
    return sqrt((a.x - b.x) * (a.x - b.x) +
            (a.y - b.y) * (a.y - b.y)
    );
}

void R_SortSectorsByDistToPlayer(vector2 player_pos)
{
    // calc sector distances
    for (int i = 0; i < queue.num_sectors; i++)
    {
        vector2 centroid = R_CalcCentroid(&queue.sectors[i]);
        queue.sectors[i].dist = R_DistanceToPoint(centroid, player_pos);
    }

    // sort sectors by distance to Player
    for (int i = 0; i < queue.num_sectors - 1; i++)
    {
        for (int j = 0; j < queue.num_sectors - i - 1; j++)
        {
            if (queue.sectors[j].dist < queue.sectors[j+1].dist)
            {
                sector s = queue.sectors[j];
                queue.sectors[j] = queue.sectors[j + 1];
                queue.sectors[j + 1] = s;
            }
        }
    }
}

void R_RenderSectors(player *Player, game_state *state, double fov)
{
    double scrn_half_w = scrnw / 2;
    double scrn_half_h = scrnh / 2;
    unsigned int wall_color = 0xFFFF00FF;

    // clear screen
    R_ClearScreenBuffer();

    // sort polygons prior processing
    R_SortSectorsByDistToPlayer(Player->position);

    // lopp sectors
    for (int i = 0; i < queue.num_sectors; i++)
    {
        sector *s = &queue.sectors[i];
        int sector_h = s->height;
        int sector_e = s->elevation;
        int sector_clr = s->color;

        for (int i = 0; i < 1024; i++)
        {
            s->ceilingx_y.t[i] = 0;
            s->ceilingx_y.b[i] = 0;
            s->floorx_y.t[i] = 0;
            s->floorx_y.b[i] = 0;
            s->portal_ceilingx_y.t[i] = 0;
            s->portal_ceilingx_y.b[i] = 0;
            s->portal_floorx_y.t[i] = 0;
            s->portal_floorx_y.b[i] = 0;
        }

        // loop walls
        for (int k = 0; k < s->num_walls; k++)
        {
            wall *w = &s->walls[k];

            // displace the world based on Player's position
            double dx1 = w->a.x - Player->position.x;
            double dy1 = w->a.y - Player->position.y;
            double dx2 = w->b.x - Player->position.x;
            double dy2 = w->b.y - Player->position.y;

            // rotate the world around the Player
            double SN = sin(Player->direction_angle);
            double CN = cos(Player->direction_angle);
            double wx1 = dx1 * SN - dy1 * CN;
            double wz1 = dx1 * CN + dy1 * SN;
            double wx2 = dx2 * SN - dy2 * CN;
            double wz2 = dx2 * CN + dy2 * SN;

            // if z1 & z2 < 0 (wall completely behind Player) -- skip it!
            // if z1 or z2 is behind the Player -> clip it!
            if (wz1 < 0 && wz2 <0)
                continue;
            else if (wz1 < 0)
                R_ClipBehindPlayer(&wx1, &wz1, wx2, wz2);
            else if (wz2 < 0)
                R_ClipBehindPlayer(&wx2, &wz2, wx1, wz1);

            // calc wall height based on distance
            double wh1 = (sector_h / wz1) * fov;
            double wh2 = (sector_h / wz2) * fov;

            // convert to screen space
            double sx1 = (wx1 / wz1) * fov;
            double sy1 = ((state->screen_h + Player->z) / wz1);
            double sx2 = (wx2 / wz2) * fov;
            double sy2 = ((state->screen_h + Player->z) / wz2);

            // calc wall elevation from the floor
            double s_level1 = (sector_e / wz1) * fov;
            double s_level2 = (sector_e / wz2) * fov;
            sy1 -= s_level1;
            sy2 -= s_level2;

            //construct portal top and bottom
            double pbh1 = 0;
            double pbh2 = 0;
            double pth1 = 0;
            double pth2 = 0;
            if (w->is_portal)
            {
                pth1 = (w->portal_top_h / wz1) * fov;
                pth2 = (w->portal_top_h / wz2) * fov;
                pbh1 = (w->portal_bottom_h / wz1) * fov;
                pbh2 = (w->portal_bottom_h / wz2) * fov;
            }

            // set screen-space origin to center of the screen
            sx1 += scrn_half_w;
            sy1 += scrn_half_h;
            sx2 += scrn_half_w;
            sy2 += scrn_half_h;

            // // top
            // R_DrawLine(sx1, sy1 - wh1, sx2, sy2 - wh2, wall_color);
            // // bottom
            // R_DrawLine(sx1, sy1, sx2, sy2, wall_color);
            // // left edge
            // R_DrawLine(sx1, sy1 - wh1, sx1, sy1, wall_color);
            // // right edge
            // R_DrawLine(sx2, sy2 - wh2, sx2, sy2, wall_color);

            // if (w->is_portal)
            // {
            //     R_DrawLine(sx1, sy1 - wh1 + pth1, sx2, sy2 - wh2 + pth2, wall_color);
            //     R_DrawLine(sx1, sy1 - pbh1, sx2, sy2 - pbh2, wall_color);
            // }

            if (w->is_portal)
            {
                // top
                rquad qt = R_CreateRendarableQuad(sx1, sx2, sy1 - wh1, sy1 - wh1 + pth1, sy2 - wh2, sy2 - wh2 + pth2);
                // bottom
                rquad qb = R_CreateRendarableQuad(sx1, sx2, sy1 - pbh1, sy1, sy2 - pbh2, sy2);

                R_Rasterize(qt, sector_clr, IS_CEILING, &s->portal_ceilingx_y);
                R_Rasterize(qt, sector_clr, IS_FLOOR, &s->portal_floorx_y);
                R_Rasterize(qt, sector_clr, IS_WALL, NULL);

                R_Rasterize(qb, sector_clr, IS_CEILING, &s->ceilingx_y);
                R_Rasterize(qb, sector_clr, IS_FLOOR, &s->floorx_y);
                R_Rasterize(qb, sector_clr, IS_WALL, NULL);
            }
            else
            {
                rquad q = R_CreateRendarableQuad(sx1, sx2, sy1 - wh1, sy1, sy2 - wh2, sy2);
                R_Rasterize(q, sector_clr, IS_CEILING, &s->ceilingx_y);
                R_Rasterize(q, sector_clr, IS_FLOOR, &s->floorx_y);
                R_Rasterize(q, sector_clr, IS_WALL, NULL);
            }
        }

        // rasterize sector's ceil & floor
        for (int x = 1; x < 1024; x++)
        {
            // walls
            int cy1 = s->ceilingx_y.t[x];
            int cy2 = s->ceilingx_y.b[x];
            int fy1 = s->floorx_y.t[x];
            int fy2 = s->floorx_y.b[x];

            // portals
            int pcy1 = s->portal_ceilingx_y.t[x];
            int pcy2 = s->portal_ceilingx_y.b[x];
            int pfy1 = s->portal_floorx_y.t[x];
            int pfy2 = s->portal_floorx_y.b[x];

            // rasterize walls ceil & floor
            if ((Player->z > s->elevation + s->height) && (cy1 > cy2) && (cy1 != 0 && cy2 != 0))
                R_DrawLine(x, cy1, x, cy2, s->ceiling_color);

            if ((Player->z < s->elevation) && (fy1 < fy2) && (fy1 != 0 || fy2 != 0))
                R_DrawLine(x, fy1, x, fy2, s->floor_color);

            // rasterize portals ceil & floor
            if (pcy1 > pcy2 && (pcy1 != 0 && pcy2 != 0))
                R_DrawLine(x, pcy1, x, pcy2, s->ceiling_color);

            if (pfy1 < pfy2 && (pfy1 != 0 || pfy2 != 0))
                R_DrawLine(x, pfy1, x, pfy2, s->floor_color);
        }
    }
}

void R_Render(player *Player, game_state *state, double fov)
{
    isDebugMode = state->isDebugMode;
    R_RenderSectors(Player, state, fov);
    R_UpdateScreen();
}

sector R_CreateSector(int height, int elevation, unsigned int color, unsigned int ceiling_color, unsigned int floor_color)
{
    static int sector_id = 0;
    sector sector = {0};
    sector.num_walls = 0;
    sector.height = height;
    sector.elevation = elevation;
    sector.color = color;
    sector.ceiling_color = ceiling_color;
    sector.floor_color = floor_color;
    sector.id = ++sector_id;

    return sector;
}

void R_SectorAddWall(sector *sector, wall vertices)
{
    sector->walls[sector->num_walls] = vertices;
    sector->num_walls++;
}

void R_AddSectorToQueue(sector *sector)
{
    queue.sectors[queue.num_sectors] = *sector;
    queue.num_sectors++;
}

wall R_CreateWall(int ax, int ay, int bx, int by)
{
    wall w;
    w.a.x = ax;
    w.a.y = ay;
    w.b.x = bx;
    w.b.y = by;
    w.is_portal = false;

    return w;
}

wall R_CreatePortal(int ax, int ay, int bx, int by, int th, int bh)
{
    wall w = R_CreateWall(ax, ay, bx, by);
    w.is_portal = true;
    w.portal_top_h = th;
    w.portal_bottom_h = bh;

    return w;
}