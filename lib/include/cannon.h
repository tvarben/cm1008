#ifndef CANNON_H
#define CANNON_H

#include "ship.h"
#include <SDL2/SDL.h>

typedef struct Cannon Cannon;

Cannon *createCannon(SDL_Renderer *renderer, int windowWidth, int windowHeight);
void drawCannon(Cannon *s);
void destroyCannon(Cannon *s);
void updateCannon(Cannon *pCannon, Ship *pShip);
void handleCannonEvent(Cannon *cannon, ClientCommand command);
void resetCannon(Cannon *c);
#endif