#ifndef CANNON_H
#define CANNON_H

#include "ship.h"
#include <SDL2/SDL.h>
typedef enum CannonDirection {UP, DOWN, LEFT, RIGHT, UP_R, UP_L, DOWN_R, DOWN_L } CannonDirection;
typedef struct Cannon Cannon;

Cannon *createCannon(SDL_Renderer *renderer, int windowWidth, int windowHeight);
void drawCannon(Cannon *s);
void destroyCannon(Cannon *s);
void updateCannon(Cannon *pCannon, Ship *pShip);
void handleCannonEvent(Cannon *c, SDL_Event *event);
void resetCannon(Cannon *c);
#endif
