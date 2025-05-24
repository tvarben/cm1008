#ifndef DATA_H
#define DATA_H

#include <stdbool.h>

#define MAX_PLAYERS 4
#define WINDOW_WIDTH 1280 // 1160
#define WINDOW_HEIGHT 720 // 700
#define SERVER_PORT 50000
#define COUNTDOWN 1
#define MAX_ENEMIES 100
#define WAVE_1_EASY_MAP 4
#define NROFBOSSES 1
#define REGULAR_DMG_GIVEN 10
#define REGULAR_DMG_TAKEN 20
#define SAVE_DATA_PATH "../lib/resources/save.txt"
#define DATA_STORED 5

typedef enum {
    START,
    ONGOING,
    GAME_OVER,
    LOBBY,
} GameState;

typedef enum {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP_LEFT,
    MOVE_UP_RIGHT,
    MOVE_DOWN_LEFT,
    MOVE_DOWN_RIGHT,
    STOP_SHIP,
    SHOOT,
    STOP_SHOOT,
    QUIT
} ClientCommand;

typedef enum {
    MAINMENU_NONE,
    MAINMENU_SINGLEPLAYER,
    MAINMENU_MULTIPLAYER,
    MAINMENU_EXIT
} MainMenuChoice;

typedef struct {
    int cDPlayerId, bulletToRemove,
    map; // cDPlayerId not really needed. Server finds out which klient
    ClientCommand command;
    bool isShooting, isAlive;
    GameState state;
    int nrOfPlayers; //decided by server
    float sessionScore;
} ClientData;

typedef struct {
    float x, y, vx, vy;
    int health, bulletToRemove; // test
    bool facingLeft, isShooting, isAlive;
} ShipData;

typedef struct {
    float x, y;
    bool active;
} Enemy_1_Data;

typedef struct {
    float x, y;
    bool active;
} Enemy_2_Data;

typedef struct {
    float x, y;
    bool active;
} Enemy_3_Data;

typedef struct {
    GameState gState;
    bool win;
    int sDPlayerId;
    ShipData ships[MAX_PLAYERS];
    int nrOfEnemies_1, nrOfEnemiesToSpawn_1;
    int nrOfEnemies_2, nrOfEnemiesToSpawn_2;
    int nrOfEnemies_3, nrOfEnemiesToSpawn_3;
    Enemy_1_Data enemies_1[MAX_ENEMIES];
    Enemy_2_Data enemies_2[MAX_ENEMIES];
    Enemy_3_Data enemies_3[MAX_ENEMIES];
    int map;
    int nrOfPlayers;
    float sessionScore;
} ServerData;

#endif
