#ifndef SHIP_H
#define SHIP_H

#include <SDL2/SDL.h>
#include "ship_data.h"
#include <stdbool.h>

typedef struct ship Ship;

Ship* createShip(int playerId, SDL_Renderer* renderer, int windowWidth, int windowHeight);
void updateShipOnClients(Ship* pShip, int shipId, int myShipId);  // NEW
void updateShipOnServer(Ship* pShip); // NEW
void stayInWindow(Ship* pShip);
void drawShip(Ship* s);
void resetShip(Ship* s);
//void handleShipEvent(Ship* s, SDL_Event* event);
void updateShipVelocity(Ship* s); 
void destroyShip(Ship* s);
void applyShipCommand(Ship* s, ClientCommand c);
void getShipDataPackage(Ship* pShip, ShipData* pShipData);
void updateShipsWithServerData(Ship *pShip, ShipData *pShipData, int shipId, int myShipId);
//void updateShipsWithServerData(Ship *pShips[MAX_PLAYERS], ShipData pShipData[MAX_PLAYERS]);
int getShipX(Ship *pShip);
int getShipY(Ship *pShip);
bool isLeft(Ship *pShip);
#endif