#ifndef PLAYER_H_
#define PLAYER_H_

#include <string>

typedef struct player
{
    char type[4];
    char name[32];
    char ip[16];
    char isHost[2];
}Player;

#endif