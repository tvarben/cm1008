#ifndef SHIP_DATA_H
#define SHIP_DATA_H

#define MAX_PLAYERS 1

typedef enum{
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    SHOOT,
    PAUSE,
    QUIT
}ClientCommand;

typedef struct{
    int playerId;
    ClientCommand command;
}ClientData;

#endif