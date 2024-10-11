#ifndef PLAYER_H
#define PLAYER_H

#include "typedefs.h"

typedef struct _player
{
    vector2 position;
    double z;
    double direction_angle;
} player;

player P_Init(double x, double y, double z, double angle);

#endif