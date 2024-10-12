#include <stdio.h>
#include <string.h>

#include "engine/player.h"
#include "engine/game_state.h"
#include "engine/window.h"
#include "engine/renderer.h"
#include "engine/keyboard.h"

#define SCREEN_W 1600
#define SCREEN_H 720 * 1.25
#define FPS 240

char *strremove(char *str, const char *sub) {
    char *p, *q, *r;
    if (*sub && (q = r = strstr(str, sub)) != NULL) {
        size_t len = strlen(sub);
        while ((r = strstr(p = r + len, sub)) != NULL) {
            while (p < r)
                *q++ = *p++;
        }
        while ((*q++ = *p++) != '\0')
            continue;
    }
    return str;
}

void GameLoop(game_state *state, player *Player)
{
    while(state->isRunning)
    {
        G_FrameStart();

        K_HandleEvents(state, Player);
        R_Render(Player, state, 350.0);

        G_FrameEnd(state);
    }
}

int main(int argc, char *argv[]) 
{   
    game_state state = G_Init(SCREEN_W, SCREEN_H, FPS);
    player Player = P_Init(40, 40, SCREEN_H * 10, M_PI/2);
    K_InitKeyMap();
    W_Init(SCREEN_W, SCREEN_H);
    R_Init(W_Get(), &state);

    char *filename = strcat(strremove(argv[0], "main.exe"), "/data/level.txt");
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        printf("Error: Could not open file %s. Make sure it's in there you dumb fucking cretin!", filename);
        return 1;
    }

    char sectorData[100];
    int sectorCount = 16;
    int nums[8];
    sector sectors[sectorCount];
    int sectorsAdditionalData[sectorCount][3];
    unsigned int colors[3];

    int numSector = 0;

    while(fgets(sectorData, 100, fp))
    {
        /*grab info from .txt file*/
        if (0==strcmp(sectorData, "[WALLS]\n"))
            break;
        else if (0!=strcmp(sectorData, "[SECTORS]\n"))
        {
            /*SECTORS*/
            
            char *token;

            int i = 0;

            token = strtok(sectorData, ",");

            while (token != NULL)
            {
                if (strstr(token, "0x"))
                    sscanf(token, "%x", &colors[i-2]);
                else
                    nums[i] = atoi(token);
                i++;
                token = strtok(NULL, ",");
            }
            //printf("[%d, %d, %u, %u, %u, %s, %d, %d]\n", nums[0], nums[1], colors[0], colors[1], colors[2], nums[5] ? "true" : "false", nums[6], nums[7]);
            sectors[numSector] = R_CreateSector(nums[0], nums[1], colors[0], colors[1], colors[2]);
            sectorsAdditionalData[numSector][0] = nums[5];
            sectorsAdditionalData[numSector][1] = nums[6];
            sectorsAdditionalData[numSector][2] = nums[7];

            numSector++;
        }
    }
    char wall_vertices[100];
    int wallCoords[sectorCount][sectorCount*4];

    int wallNum = 0;
    int secNum = -1;

    while(fgets(wall_vertices, 100, fp))
    {
        if (strstr(wall_vertices, "# SECTOR: "))
        {
            secNum++; wallNum = 0;
            strremove(wall_vertices, "# SECTOR: ");
        }

        char *token;

        token = strtok(wall_vertices, ",");

        while( token != NULL ) 
        {
            wallCoords[secNum][wallNum] = atoi(token);
            wallNum++;
            token = strtok(NULL, ",");
        }
    }   

    fclose(fp);

    int numOfWalls[] = {16, 16, 16, 32, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};

    for (int i = 0; i < sectorCount; i++)
    {
        for (int j = 0; j < numOfWalls[i]; j += 4)
        {
            //printf("[%d, %d, %d, %d]\n", wallCoords[i][j], wallCoords[i][j+1], wallCoords[i][j+2], wallCoords[i][j+3]);
            wall w;
            if (sectorsAdditionalData[i][0] == 1) {
                w = R_CreatePortal(wallCoords[i][j], wallCoords[i][j + 1], wallCoords[i][j + 2], wallCoords[i][j + 3], sectorsAdditionalData[i][1], sectorsAdditionalData[i][2]);
            }
            else
            {
                w = R_CreateWall(wallCoords[i][j], wallCoords[i][j + 1], wallCoords[i][j + 2], wallCoords[i][j + 3]);
            }
            R_SectorAddWall(&sectors[i], w);
        }
        R_AddSectorToQueue(&sectors[i]);
    }

    GameLoop(&state, &Player);

    return 0;
}