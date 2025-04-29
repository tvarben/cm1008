#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "ship.h"
#include "ship_data.h"

struct ship {
    float x, y, vx, vy, xStart, yStart; //x och y används inte? kolla rad 50
    //int vx, vy;
    float targetX, targetY; // for smooth movement
    int windowWidth, windowHeight;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Rect shipRect;
    bool keyLeft, keyRight, keyUp, keyDown;
};

Ship* createShip(int playerId, SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    Ship* pShip = malloc(sizeof(Ship));
    if (!pShip) return NULL;

    pShip->vx = 0;
    pShip->vy = 0;
    pShip->windowWidth = windowWidth;
    pShip->windowHeight = windowHeight;
    pShip->renderer = renderer;
    pShip->keyDown=pShip->keyUp=pShip->keyRight=pShip->keyLeft=false;

    SDL_Surface* surface = IMG_Load("../lib/resources//player.png");
    if (!surface) {
        printf("Error loading player.png: %s\n", IMG_GetError());
        free(pShip);
        return NULL;
    }

    pShip->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Free the surface after creating the texture to be readyu for the next picture

    if (!pShip->texture) {
        printf("Error creating texture: %s\n", SDL_GetError());
        free(pShip);
        return NULL;
    }

    SDL_QueryTexture(pShip->texture, NULL, NULL, &pShip->shipRect.w, &pShip->shipRect.h);
    pShip->shipRect.w /= 4;
    pShip->shipRect.h /= 4;
    pShip->xStart /*= pShip->x */= pShip->shipRect.x = windowWidth/2-pShip->shipRect.h/2;
    pShip->yStart /*= pShip->y */= pShip->shipRect.y = (playerId*100) + 50;
    // For smooth movement, initialize target position to start position
    pShip->targetX = pShip->xStart;
    pShip->targetY = pShip->yStart;
    return pShip;
}

void handleShipEvent(Ship* pShip, SDL_Event* event) {
    bool down = event->type == SDL_KEYDOWN;

    switch (event->key.keysym.scancode) {
        case SDL_SCANCODE_W: 
        case SDL_SCANCODE_UP:
            pShip->keyUp = down; break;
        case SDL_SCANCODE_S: 
        case SDL_SCANCODE_DOWN:
            pShip->keyDown = down; break;
        case SDL_SCANCODE_A: 
        case SDL_SCANCODE_LEFT:
            pShip->keyLeft = down; break;
        case SDL_SCANCODE_D: 
        case SDL_SCANCODE_RIGHT:
            pShip->keyRight = down; break;
        default: break;
    }
}

void setShipVelocity(Ship* pShip, int vx, int vy) {
    pShip->vx = vx;
    pShip->vy = vy;
}

void updateShipVelocity(Ship* pShip) {
    int vx = 0, vy = 0;

    if (pShip->keyLeft && !pShip->keyRight) vx = -1;
    else if (pShip->keyRight && !pShip->keyLeft) vx = 1;

    if (pShip->keyUp && !pShip->keyDown) vy = -1;
    else if (pShip->keyDown && !pShip->keyUp) vy = 1;

    pShip->vx = vx;
    pShip->vy = vy;
}

// For client-side prediction + interpolation
void updateShip(Ship* pShip, int shipId, int myShipId) {
    const int speed = 4;
    
    if (shipId == myShipId) {
        pShip->xStart += pShip->vx * speed;
        pShip->yStart += pShip->vy * speed;
    } else {
        const float lerpFactor = 0.15f;
        pShip->xStart += (pShip->targetX - pShip->xStart) * lerpFactor;
        pShip->yStart += (pShip->targetY - pShip->yStart) * lerpFactor;
    }

    pShip->shipRect.x = (int)pShip->xStart;
    pShip->shipRect.y = (int)pShip->yStart;

     // BOUNDARY CHECK 
     if (pShip->xStart < 0) pShip->xStart = 0;
     if (pShip->yStart < 0) pShip->yStart = 0;
     if (pShip->xStart > pShip->windowWidth - pShip->shipRect.w) 
         pShip->xStart = pShip->windowWidth - pShip->shipRect.w;
     if (pShip->yStart > pShip->windowHeight - pShip->shipRect.h) 
         pShip->yStart = pShip->windowHeight - pShip->shipRect.h;

}

// For server (no client prediction, pure physics)
void updateShipServer(Ship* pShip) {
    const int speed = 4;

    pShip->xStart += pShip->vx * speed;
    pShip->yStart += pShip->vy * speed;

    pShip->shipRect.x = (int)pShip->xStart;
    pShip->shipRect.y = (int)pShip->yStart;

    // BOUNDARY CHECK
    if (pShip->xStart < 0) pShip->xStart = 0;
    if (pShip->yStart < 0) pShip->yStart = 0;
    if (pShip->xStart > pShip->windowWidth - pShip->shipRect.w) 
        pShip->xStart = pShip->windowWidth - pShip->shipRect.w;
    if (pShip->yStart > pShip->windowHeight - pShip->shipRect.h) 
        pShip->yStart = pShip->windowHeight - pShip->shipRect.h;
}

void drawShip(Ship* pShip) {
    SDL_RenderCopy(pShip->renderer, pShip->texture, NULL, &pShip->shipRect);
}

void resetShip(Ship* pShip) {
    //pShip->xStart = pShip->windowWidth / 2.0f;
    //pShip->yStart = pShip->windowHeight / 2.0f;
    pShip->vx = 0;
    pShip->vy = 0;
    pShip->keyRight = pShip->keyLeft = pShip->keyDown = pShip->keyUp = false;
}

void destroyShip(Ship* pShip) {
    if (pShip) {
        if (pShip->texture) SDL_DestroyTexture(pShip->texture);
        free(pShip);
    }
}

void applyShipCommand(Ship* pShip, ClientCommand c) {
    switch (c) {
        case STOP_SHIP:
            pShip->keyDown = pShip->keyUp = pShip->keyLeft = pShip->keyRight = false;
            break;
        case MOVE_UP:
            pShip->keyUp = true;
            pShip->keyDown = false;
            break;
        case MOVE_DOWN:
            pShip->keyDown = true;
            pShip->keyUp = false;
            break;
        case MOVE_LEFT:
            pShip->keyLeft = true;
            pShip->keyRight = false;
            break;
        case MOVE_RIGHT:
            pShip->keyRight = true;
            pShip->keyLeft = false;
            break;
        case SHOOT:
        case QUIT:
        default:
            break;
    }
}    

void getShipDataPackage(Ship* pShip, ShipData* pShipData) {
        pShipData->x = pShip->xStart;
        pShipData->y = pShip->yStart;
        pShipData->vx = pShip->vx;
        pShipData->vy = pShip->vy;
}

void updateShipsWithServerData(Ship *pShip, ShipData *pShipData, int shipId, int myShipId) {
    if (shipId == myShipId) return;
    pShip->targetX = pShipData->x;
    pShip->targetY = pShipData->y;
    pShip->vx = pShipData->vx;
    pShip->vy = pShipData->vy;
}

/*void updateShipsWithServerData(Ship *pShips[MAX_PLAYERS], ShipData pShipData[MAX_PLAYERS]) {
    for(int i = 0; i < MAX_PLAYERS; i++) {
        if (pShips[i]) {
            pShips[i]->targetX  = pShipData[i].x;  // <- use xStart, not x
            pShips[i]->targetY = pShipData[i].y;
            //pShips[i]->shipRect.x = (int)pShipData[i].x;
            //pShips[i]->shipRect.y = (int)pShipData[i].y;
        }
    }
}*/