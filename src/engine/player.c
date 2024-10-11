#include "player.h"

player P_Init(double x, double y, double z, double angle)
{
    player newPlayer;
    newPlayer.position.x = x;
    newPlayer.position.x = y;
    newPlayer.z = z;
    newPlayer.direction_angle = angle;

    return newPlayer;
}