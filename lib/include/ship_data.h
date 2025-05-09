#ifndef SHIP_DATA_H
#define SHIP_DATA_H

#define MAX_PLAYERS 2
#define WINDOW_WIDTH 1160 //1160
#define WINDOW_HEIGHT 700 //700
#define SERVER_PORT 2000
#define COUNTDOWN 1
#define MAX_ENEMIES 100
#define WAVE_1_EASY_MAP 4

typedef enum {
    START, 
    ONGOING, 
    GAME_OVER,
    LOBBY, 
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

typedef enum {
    MAINMENU_NONE,
    MAINMENU_SINGLEPLAYER,
    MAINMENU_MULTIPLAYER,
    MAINMENU_EXIT
} MainMenuChoice;

typedef struct{
    int cDPlayerId;
    ClientCommand command;
}ClientData;

typedef struct{
    float x, y, vx, vy;
    //int vx, vy;   //Is vx, vy needed?
}ShipData;

typedef struct{
    float x, y;
    bool active;
} Enemy_1_Data;

typedef struct{
    float x, y;
    bool active;
} Enemy_2_Data;

typedef struct{
    GameState gState;
    int sDPlayerId;
    ShipData ships[MAX_PLAYERS];
    
    int nrOfEnemies_1, nrOfEnemiesToSpawn_1;
    //int nrOfEnemies_2, nrOfEnemiesToSpawn_2;

    Enemy_1_Data enemies_1[MAX_ENEMIES];
    //Enemy_2_Data enemies_2[MAX_ENEMIES];
} ServerData;



#endif