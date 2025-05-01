#ifndef SHIP_DATA_H
#define SHIP_DATA_H

#define MAX_PLAYERS 2
#define WINDOW_WIDTH 1280 //1160
#define WINDOW_HEIGHT 720 //700
#define SERVER_PORT 50000

typedef enum {
    START, 
    ONGOING, 
    GAME_OVER,
    MULTIPLAYER 
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
    GameState gState;
    int sDPlayerId;
    ShipData ships[MAX_PLAYERS];
}ServerData;

#endif