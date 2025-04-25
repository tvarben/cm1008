#ifndef rocket_data_h
#define rocket_data_h

#define WINDOW_WIDTH 900//1280
#define WINDOW_HEIGHT 560//960
#define MAX_ROCKETS 4

enum gameState{START, ONGOING, GAME_OVER};
typedef enum gameState GameState;

enum clientCommand{READY, ACC, LEFT, RIGHT, FIRE};
typedef enum clientCommand ClientCommand;

struct clientData{
    ClientCommand command;
    int playerNumber;
};
typedef struct clientData ClientData;

struct bulletData{
    float x, y, vx, vy;
    int time;
};
typedef struct bulletData BulletData;

struct rocketData{
    float x, y, vx, vy;
    int angle, alive;
    BulletData bData;
};
typedef struct rocketData RocketData;

struct serverData{
    RocketData rockets[MAX_ROCKETS];
    int playerNr;
    GameState gState;
};
typedef struct serverData ServerData;

#endif