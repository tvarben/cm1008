#include "ship.h"
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
float distance(int x1, int y1, int x2, int y2);

struct Ship {
    float x, y;
    int vx, vy;
    int windowWidth, windowHeight;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Rect rect;
    bool keyLeft, keyRight, keyUp, keyDown;
    float rotationAngle;
    bool facingLeft;
    SDL_Rect hitbox; 
    int health;
};
Ship* createShip(int x, int y, SDL_Renderer* renderer, int windowWidth, int windowHeight) {
    Ship* s = malloc(sizeof(Ship));
    if (!s) return NULL;

    s->vx = 0;
    s->vy = 0;
    s->health=2;
    s->windowWidth = windowWidth;
    s->windowHeight = windowHeight;
    s->renderer = renderer;
    s->rotationAngle = 0.0f;  // Initialize rotation angle to 0

    SDL_Surface* surface = IMG_Load("resources/player.png");
    if (!surface) {
        printf("Error loading player.png: %s\n", IMG_GetError());
        free(s);
        return NULL;
    }

    s->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!s->texture) {
        printf("Error creating texture: %s\n", SDL_GetError());
        free(s);
        return NULL;
    }

    SDL_QueryTexture(s->texture, NULL, NULL, &s->rect.w, &s->rect.h);
    s->rect.w /= 4;
    s->rect.h /= 4;

    s->x = x - s->rect.w / 2;
    s->y = y - s->rect.h / 2;
    s->rect.x = (int)s->x;
    s->rect.y = (int)s->y;

    return s;
}

void handleShipEvent(Ship* s, SDL_Event* event) {
    bool down = event->type == SDL_KEYDOWN;

    switch (event->key.keysym.scancode) {
        case SDL_SCANCODE_W: case SDL_SCANCODE_UP:
            s->keyUp = down; break;
        case SDL_SCANCODE_S: case SDL_SCANCODE_DOWN:
            s->keyDown = down; break;
        case SDL_SCANCODE_A: case SDL_SCANCODE_LEFT:
            s->keyLeft = down; break;
        case SDL_SCANCODE_D: case SDL_SCANCODE_RIGHT:
            s->keyRight = down; break;
        default: break;
    }
}
void setShipVelocity(Ship* s, int vx, int vy) {
    s->vx = vx;
    s->vy = vy;
}
void updateShipVelocity(Ship* s) {
    int vx = 0, vy = 0;

    if (s->keyLeft && !s->keyRight) {
        vx = -1;
        s->facingLeft = true;
    } else if (s->keyRight && !s->keyLeft) {
        vx = 1;
        s->facingLeft = false;
    }

    if (s->keyUp && !s->keyDown) vy = -1;
    else if (s->keyDown && !s->keyUp) vy = 1;

    s->vx = vx;
    s->vy = vy;
}

void updateShip(Ship* s) {
    const int speed = 4; // constant speed
    s->x += s->vx * speed;
    s->y += s->vy * speed;

    // Stay within bounds
    if (s->x < 0) s->x = 0;
    if (s->y < 0) s->y = 0;
    if (s->x > s->windowWidth - s->rect.w) s->x = s->windowWidth - s->rect.w;
    if (s->y > s->windowHeight - s->rect.h) s->y = s->windowHeight - s->rect.h;

    // Update rotation angle based on movement direction
    if (s->vx < 0) {
        s->rotationAngle = 180.0f;  // Faces left when moving left
    } else if (s->vx > 0) {
        s->rotationAngle = 0.0f;  // Faces right when moving right
    }

    s->rect.x = (int)s->x;
    s->rect.y = (int)s->y;
    
    s->hitbox.w = s->rect.w * 0.7;
    s->hitbox.h = s->rect.h * 0.2;
    s->hitbox.x = s->x + (s->rect.w - s->hitbox.w) / 2;
    s->hitbox.y = s->y + (s->rect.h - s->hitbox.h) / 2;
}


void drawShip(Ship* s) {
    SDL_RendererFlip flip = s->facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(s->renderer, s->texture, NULL, &s->rect, 0, NULL, flip);
}
void resetShip(Ship* s) {
    s->x = s->windowWidth / 2.0f;
    s->y = s->windowHeight / 2.0f;
    s->vx = 0;
    s->vy = 0;
    s->keyUp = false;
    s->keyDown = false;
    s->keyLeft = false;
    s->keyRight = false;
}

void destroyShip(Ship* s) {
    if (s) {
        if (s->texture) SDL_DestroyTexture(s->texture);
        free(s);
    }
}

int getShipX(Ship *s) { return s->x; }
int getShipY(Ship *s) { return s->y; }


int shipCollision(Ship *pShip, SDL_Rect rect) {
    return SDL_HasIntersection(&pShip->rect, &rect);
}


float distance(int x1, int y1, int x2, int y2) {
    return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

bool isLeft(Ship *pShip)
{
    if (pShip->facingLeft == true )
    {
        return true;
    }
    if (pShip->facingLeft == false )
    {
        return false;
    }
}

void damageShip(Ship *pShip)
{
    pShip->health -=1;
    printf("Ship health %d\n",pShip->health);
}

bool isPlayerDead(Ship *pShip)
{
    if (pShip->health <= 0)
    {
        return true;
    }
    else
    {
        return false;
    }
    
}
void resetHealth(Ship *pShip)
{
    pShip->health=2;
    return;
}