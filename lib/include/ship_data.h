#ifndef SHIP_DATA_H
#define SHIP_DATA_H

#define MAX_PLAYERS 2

typedef enum {
    START, 
    ONGOING, 
    GAME_OVER 
}GameState;

typedef enum{
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    STOP_SHIP,
    SHOOT,
    QUIT
}ClientCommand;

typedef struct{
    int cDPlayerId;
    ClientCommand command;
}ClientData;

typedef struct{
    float x, y, vx, vy;
    //int vx, vy;   //Is vx, vy needed?
}ShipData;

typedef struct{
    ShipData ships[MAX_PLAYERS];
    int sDPlayerId;
    GameState gState;
}ServerData;

#endif