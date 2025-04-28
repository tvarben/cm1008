#ifndef SHIP_H
#define SHIP_H

#include <SDL2/SDL.h>
#include "ship_data.h"

typedef struct ship Ship;
/*typedef struct {
    float x, y, vx, vy, xStart, yStart; //x och y används inte? kolla rad 50
    //int vx, vy;
    int windowWidth, windowHeight;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Rect shipRect;
    bool keyLeft, keyRight, keyUp, keyDown;
}Ship;*/

Ship* createShip(int playerId, SDL_Renderer* renderer, int windowWidth, int windowHeight);
void setShipVelocity(Ship* s, int vx, int vy);  // NEW
void updateShip(Ship* s);
void drawShip(Ship* s);
void resetShip(Ship* s);
void handleShipEvent(Ship* s, SDL_Event* event);
void updateShipVelocity(Ship* s); 
void destroyShip(Ship* s);
void applyShipCommand(Ship* s, ClientCommand c);
void getShipDataPackage(Ship* pShip, ShipData* pShipData);
//void updateShipsWithServerData(Ship *pShip, ShipData *pShipData);
void updateShipsWithServerData(Ship *pShips[MAX_PLAYERS], const ShipData *pShipData[MAX_PLAYERS]);
#endif
