#include "../include/ship.h"
#include "../include/ship_data.h"
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SPEED 4

struct ship {
    float x, y, vx, vy, xStart, yStart, targetX,
        targetY; // x och y används inte? kolla rad 50
    // float targetX, targetY; // for smooth movement
    int windowWidth, windowHeight;
    SDL_Renderer *renderer;
    SDL_Texture *texture, *shield;
    SDL_Rect shipRect, shieldRect;
    int health, bulletToRemove;
    bool keyLeft, keyRight, keyUp, keyDown, facingLeft, isShooting, isAlive;
};

Ship *createShip(int playerId, SDL_Renderer *renderer, int windowWidth, int windowHeight) {
    Ship *pShip = malloc(sizeof(Ship));
    if (!pShip) return NULL;

    pShip->vx = 0;
    pShip->vy = 0;
    pShip->windowWidth = windowWidth;
    pShip->windowHeight = windowHeight;
    pShip->renderer = renderer;
    pShip->keyDown = pShip->keyUp = pShip->keyRight = pShip->keyLeft = pShip->isShooting = false;
    pShip->health = 100;
    pShip->isAlive = true;

    // SDL_Surface *surface = IMG_Load("../lib/resources/player.png");
    char imagePath[64];
    sprintf(imagePath, "../lib/resources/player%d.png", playerId);
    SDL_Surface *surface = IMG_Load(imagePath);
    if (!surface) {
        printf("Error loading player.png: %s\n", IMG_GetError());
        free(pShip);
        return NULL;
    }

    pShip->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Free the surface after creating the texture to be
                              // readyu for the next picture

    if (!pShip->texture) {
        printf("Error creating ship texture: %s\n", SDL_GetError());
        free(pShip);
        return NULL;
    }

    SDL_QueryTexture(pShip->texture, NULL, NULL, &pShip->shipRect.w, &pShip->shipRect.h);
    pShip->shipRect.w /= 4;
    pShip->shipRect.h /= 4;
    pShip->xStart = pShip->x = pShip->shipRect.x = windowWidth / 2 - pShip->shipRect.h / 2;
    pShip->yStart = pShip->y = pShip->shipRect.y = (playerId * 100) + 50;
    // For smooth movement, initialize target position to start position
    pShip->targetX = pShip->xStart;
    pShip->targetY = pShip->yStart;

    SDL_Surface *surface2 = IMG_Load("../lib/resources/shield.png");
    if (!surface) {
        printf("Error loading shield.png: %s\n", IMG_GetError());
        free(pShip);
        return NULL;
    }

    pShip->shield = SDL_CreateTextureFromSurface(renderer, surface2);
    SDL_FreeSurface(surface2);

    if (!pShip->shield) {
        printf("Error creating shield texture: %s\n", SDL_GetError());
        free(pShip);
        return NULL;
    }
    SDL_QueryTexture(pShip->shield, NULL, NULL, NULL, NULL);
    pShip->shieldRect.w = (pShip->shipRect.w) * 1.5;  // made the shield wider
    pShip->shieldRect.h = (pShip->shipRect.h) * 2.25; // made the ship taller

    pShip->shieldRect.x = pShip->x;
    pShip->shieldRect.y = pShip->y;
    SDL_SetTextureAlphaMod(pShip->shield, 150); // sets transparency level, lower means more transparent

    return pShip;
}

void setShipVelocity(Ship *pShip, int vx, int vy) {
    pShip->vx = vx;
    pShip->vy = vy;
}

void updateShipVelocity(Ship *pShip) {
    int vx = 0, vy = 0;

    if (pShip->keyLeft && !pShip->keyRight) {
        vx = -1;
        pShip->facingLeft = true;
    } else if (pShip->keyRight && !pShip->keyLeft) {
        vx = 1;
        pShip->facingLeft = false;
    }

    if (pShip->keyUp && !pShip->keyDown)
        vy = -1;
    else if (pShip->keyDown && !pShip->keyUp)
        vy = 1;

    pShip->vx = vx;
    pShip->vy = vy;
}

// For client-side prediction + interpolation
void updateShipOnClients(Ship *pShip, int shipId, int myShipId) {
    const float lerpFactor = 0.75f; // 75% Adjust this value to control the interpolation speed, the
                                    // closer to 1 the faster
    const float correctionFactor = 5; // nr of pixels, threshold for correction
    float dx, dy;

    if (shipId == myShipId) {
        pShip->x += pShip->vx * SPEED;
        pShip->y += pShip->vy * SPEED;
        dx = pShip->targetX - pShip->x;
        dy = pShip->targetY - pShip->y;

        if (dx < -correctionFactor || dx > correctionFactor || dy < -correctionFactor ||
            dy > correctionFactor) {
            pShip->x += (dx)*lerpFactor;
            pShip->y += (dy)*lerpFactor;
        }
    } else {
        pShip->x += (pShip->targetX - pShip->x) * lerpFactor;
        pShip->y += (pShip->targetY - pShip->y) * lerpFactor;
    }
    stayInWindow(pShip);
}

bool isLeft(Ship *pShip) {
    if (pShip->facingLeft == true) {
        return true;
    }
    if (pShip->facingLeft == false) {
        return false;
    }
}

int getShipX(Ship *pShip) {
    return pShip->x;
}
int getShipY(Ship *pShip) {
    return pShip->y;
}

// For server (no client prediction, pure physics)
void updateShipOnServer(Ship *pShip) {

    pShip->x += pShip->vx * SPEED;
    pShip->y += pShip->vy * SPEED;
    /*pShip->shipRect.x += pShip->vx * SPEED;*/
    /*pShip->shipRect.y += pShip->vy * SPEED;*/

    stayInWindow(pShip);
}

void stayInWindow(Ship *pShip) {
    pShip->shipRect.x = (int)pShip->x;
    pShip->shipRect.y = (int)pShip->y;
    if (pShip->facingLeft) {
        pShip->shieldRect.x = ((int)pShip->x) - 17; // not sure if this is more left or more right
    } else {
        pShip->shieldRect.x = ((int)pShip->x) - 18;
    }
    pShip->shieldRect.y = ((int)pShip->y) - 15; // adjusted where shield is placed, -15 is a bit up

    // BOUNDARY CHECK
    if (pShip->x < 0) pShip->x = 0;
    if (pShip->y < 0) pShip->y = 0;
    if (pShip->x > pShip->windowWidth - pShip->shipRect.w)
        pShip->x = pShip->windowWidth - pShip->shipRect.w;
    if (pShip->y > pShip->windowHeight - pShip->shipRect.h)
        pShip->y = pShip->windowHeight - pShip->shipRect.h;
}

void drawShip(Ship *pShip) {
    SDL_RendererFlip flip = pShip->facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    if (pShip->health <= 0) { //
        pShip->shipRect.w = 0;
        pShip->shipRect.h = 0;
        return;
    }
    SDL_RenderCopyEx(pShip->renderer, pShip->texture, NULL, &pShip->shipRect, 0, NULL, flip);
    if (pShip->health == 100) {
        SDL_RenderCopy(pShip->renderer, pShip->shield, NULL, &pShip->shieldRect);
    }
    else if (pShip->health == 80) {
        SDL_SetTextureAlphaMod(pShip->shield, 120); // sets transparency level, lower means more transparent
        SDL_RenderCopy(pShip->renderer, pShip->shield, NULL, &pShip->shieldRect);
    }
    else if (pShip->health == 60) {
        SDL_SetTextureAlphaMod(pShip->shield, 90); // sets transparency level, lower means more transparent
        SDL_RenderCopy(pShip->renderer, pShip->shield, NULL, &pShip->shieldRect);
    }
    else if (pShip->health == 40) {
         SDL_SetTextureAlphaMod(pShip->shield, 60); // sets transparency level, lower means more transparent
        SDL_RenderCopy(pShip->renderer, pShip->shield, NULL, &pShip->shieldRect);
    }
}

void resetShip(Ship *pShip, int playerId) {
    pShip->isAlive = true;
    pShip->vx = 0;
    pShip->vy = 0;
    pShip->keyRight = pShip->keyLeft = pShip->keyDown = pShip->keyUp = false;
    pShip->shipRect.w /= 4;
    pShip->shipRect.h /= 4;
    pShip->xStart = pShip->x = pShip->shipRect.x = WINDOW_HEIGHT / 2 - pShip->shipRect.h / 2;
    pShip->yStart = pShip->y = pShip->shipRect.y = (playerId * 100) + 50;
}

void destroyShip(Ship *pShip) {
    if (pShip) {
        if (pShip->texture) {
            SDL_DestroyTexture(pShip->texture);
        }
        if (pShip->shield) {
            SDL_DestroyTexture(pShip->shield);
        }
        free(pShip);
    }
}

void applyShipCommand(Ship *pShip, ClientCommand command) {
    pShip->keyDown = pShip->keyUp = pShip->keyLeft = pShip->keyRight = false;
    switch (command) {
    case STOP_SHIP:
        pShip->keyDown = pShip->keyUp = pShip->keyLeft = pShip->keyRight = false;
        break;
    case MOVE_UP:
        pShip->keyUp = true;
        printf("ship health: %d \n", pShip->health);
        break;
    case MOVE_DOWN:
        pShip->keyDown = true;
        break;
    case MOVE_LEFT:
        pShip->keyLeft = true;
        break;
    case MOVE_RIGHT:
        pShip->keyRight = true;
        break;
    case MOVE_UP_LEFT:
        pShip->keyUp = true;
        pShip->keyLeft = true;
        break;
    case MOVE_UP_RIGHT:
        pShip->keyUp = true;
        pShip->keyRight = true;
        break;
    case MOVE_DOWN_LEFT:
        pShip->keyDown = true;
        pShip->keyLeft = true;
        break;
    case MOVE_DOWN_RIGHT:
        pShip->keyDown = true;
        pShip->keyRight = true;
        break;
    case QUIT:
    default:
        break;
    }
}

void getShipDataPackage(Ship *pShip,
                        ShipData *pShipData) { // retrieves data from server
    pShipData->x = pShip->x;
    pShipData->y = pShip->y;
    pShipData->vx = pShip->vx;
    pShipData->vy = pShip->vy;
    pShipData->health = pShip->health;
    pShipData->facingLeft = pShip->facingLeft;
    pShipData->isShooting = pShip->isShooting;
    pShipData->isAlive = pShip->isAlive;
    pShipData->bulletToRemove = pShip->bulletToRemove;
}

void updateShipsWithServerData(Ship *pShip, ShipData *pShipData, int shipId, int myShipId) {
    pShip->targetX = pShipData->x;
    pShip->targetY = pShipData->y;
    pShip->vx = pShipData->vx;
    pShip->vy = pShipData->vy;
    pShip->health = pShipData->health;
    pShip->facingLeft = pShipData->facingLeft;
    pShip->isShooting = pShipData->isShooting;
    pShip->isAlive = pShipData->isAlive;
    pShip->bulletToRemove = pShipData->bulletToRemove;
}

bool isCannonShooting(Ship *pShip) {
    return pShip->isShooting;
}

void setShoot(Ship *pShip, bool value) {
    pShip->isShooting = value;
}

int shipCollision(Ship *pShip, SDL_Rect rect) {
    return SDL_HasIntersection(&pShip->shipRect, &rect);
}

void damageShip(Ship *pShip, int damage) {
    pShip->health -= damage;
    // printf("Ship health %d\n", pShip->health);
}

bool isPlayerDead(Ship *pShip) {
    if (pShip->health <= 0) {
        pShip->isAlive = false;
        return true;
    } else {
        return false;
    }
}

void resetHealth(Ship *pShip) {
    pShip->health = 100;
    return;
}

bool clientAliveControll(Ship *pShip) {
    if (pShip->isAlive == false) {
        return false;
    } else {
        return true;
    }
}
void setBulletToRemove(Ship *pShip, int bulletToRemove) {
    pShip->bulletToRemove = bulletToRemove;
}

int getBulletToRemove(Ship *pShip) {
    int bullet;
    if (pShip) bullet = pShip->bulletToRemove;
    pShip->bulletToRemove = -1;
    return bullet;
}