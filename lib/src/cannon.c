#include "../include/cannon.h"
#include "../include/bullet.h"
#include "../include/ship.h"
#include "../include/ship_data.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct Cannon {
    int dy, dx, windowWidth, windowHeight, health;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Rect rect;
    bool spacebar, moveLeftQ, moveRightE, moveDownN; // keys decide direction of bullet
    bool hpUpgradeLockedIn, lastFacedLeft;
};

Cannon *createCannon(SDL_Renderer *renderer, int windowWidth, int windowHeight) {
    Cannon *c = malloc(sizeof(Cannon));
    if (!c) return NULL;

    c->windowWidth = windowWidth;
    c->windowHeight = windowHeight;
    c->renderer = renderer;
    c->dy = 0; // direction cannon shoots when press space at start of the game
    c->dx = 5;
    c->lastFacedLeft = false;
    c->health = 100;
    c->hpUpgradeLockedIn = false;
    SDL_Surface *surface = IMG_Load("../lib/resources/ship_cannon.png");
    if (!surface) {
        printf("Error loading Cannon.png: %s\n", IMG_GetError());
        free(c);
        return NULL;
    }

    c->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!c->texture) {
        printf("Error creating texture: %s\n", SDL_GetError());
        free(c);
        return NULL;
    }

    SDL_QueryTexture(c->texture, NULL, NULL, &c->rect.w, &c->rect.h);
    c->rect.w /= 2; // width of image
    c->rect.h /= 2; // height of image

    return c;
}

void drawCannon(Cannon *c) {
    SDL_RendererFlip flip = c->lastFacedLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    if (c->health <= 0) {
        return;
    }
    SDL_RenderCopyEx(c->renderer, c->texture, NULL, &c->rect, 0, NULL, flip);
}

void destroyCannon(Cannon *c) {
    if (c) {
        if (c->texture) SDL_DestroyTexture(c->texture);
        free(c);
    }
}

void updateCannon(Cannon *pCannon, Ship *pShip) {
    double cannonLocationX = 28;
    double cannonLocationY = 15;

    pCannon->rect.y = getShipY(pShip) + cannonLocationY;
    pCannon->rect.x = getShipX(pShip) + cannonLocationX;

    if (isLeft(pShip)) {
        pCannon->lastFacedLeft = true;
        pCannon->dx = -5;
        pCannon->dy = 0;
    } else {
        pCannon->lastFacedLeft = false;
        pCannon->dx = 5;
        pCannon->dy = 0;
    }
}

void handleCannonEvent(Cannon *cannon) {
    if (cannon->lastFacedLeft) {
        spawn_projectile(cannon->rect.x - 8, cannon->rect.y + 15, cannon->dx, 0);
    } else {
        spawn_projectile(cannon->rect.x + 20, cannon->rect.y + 15, cannon->dx, 0);
    }
}

void resetCannon(Cannon *c) {
    c->spacebar = false;
    c->moveDownN = false;
    c->moveLeftQ = false;
    c->moveRightE = false;
}

void damageCannon(Cannon *pCannon, int damage) {
    pCannon->health -= damage;
}

void resetCannonHealth(Cannon *pCannon) {
    pCannon->health = 100;
}

void cannonHpUpgrade(Cannon *pCannon) {
    if (pCannon->hpUpgradeLockedIn == false) {
        pCannon->health = 200;
        pCannon->hpUpgradeLockedIn = true;
    }
}
