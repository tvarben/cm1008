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
    int playerId;
    ClientCommand command;
}ClientData;

typedef struct{
    float x, y;
    //int vx, vy;   //Is vx, vy needed?
}ShipData;

typedef struct{
    ShipData ships[MAX_PLAYERS];
    int playerId;
    GameState gState;
}ServerData;

#endif