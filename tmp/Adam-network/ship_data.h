#ifndef SHIP_DATA_H
#define SHIP_DATA_H

#define MAX_PLAYERS 2

typedef enum {
    START,
    RUNNING,
    GAME_OVER
} GameState;

typedef enum {
    CMD_NONE,
    CMD_UP,
    CMD_DOWN,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_FIRE
} Command;

typedef struct {
    int playerNumber;
    Command command;
} ClientData;

typedef struct {
    int playerNr;
    GameState gState;
    struct {
        float x, y;
        int vx, vy;
    } ships[MAX_PLAYERS];
} ServerData;

#endif