#ifndef SHIP_H
#define SHIP_H

#include <SDL2/SDL.h>

typedef struct Ship Ship;

Ship* createShip(int x, int y, SDL_Renderer* renderer, int windowWidth, int windowHeight);
void setShipVelocity(Ship* s, int vx, int vy);  // NEW
void updateShip(Ship* s);
void drawShip(Ship* s);
void resetShip(Ship* s);
void handleShipEvent(Ship* s, SDL_Event* event);
void updateShipVelocity(Ship* s); 
void destroyShip(Ship* s);

//getters and setters for ship properties
float getShipX(Ship* s);
float getShipY(Ship* s);
void setShipPosition(Ship* s, float x, float y);
int getShipVx(Ship* s);
int getShipVy(Ship* s);


#endif
