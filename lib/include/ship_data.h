#ifndef SHIP_DATA_H
#define SHIP_DATA_H

#define WINDOW_WIDTH
#define MAX_PLAYERS 1

/*typedef enum {
	START,
	ONGOING,
	GACE_OVER
};
typedef enum gameState GameState;*/

typedef enum{
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    SHOOT,
    PAUSE,
    QUIT
    }Command;
//typedef enum command Command;

typedef struct{
    int player_id;
    Command command;
}ClientCommand;

#endif