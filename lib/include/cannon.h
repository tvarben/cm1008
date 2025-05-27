#ifndef CANNON_H
#define CANNON_H

#include "ship.h"
#include <SDL2/SDL.h>

typedef struct Cannon Cannon;

Cannon *createCannon(SDL_Renderer *renderer, int windowWidth, int windowHeight);
void drawCannon(Cannon *s);
void destroyCannon(Cannon *s);
void updateCannon(Cannon *pCannon, Ship *pShip);
void handleCannonEvent(Cannon *cannon);
void resetCannon(Cannon *c);
void applyCannonCommand(Cannon *pCannon, ClientCommand command);
void damageCannon(Cannon *pCannon, int damage);
void resetCannonHealth(Cannon *pCannon);
void cannonHpUpgrade(Cannon *pCannon);
#endif