#include "../include/cannon.h"
#include "../include/bullet.h"
#include "ship.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct Cannon {
  int dy, dx;
  int windowWidth, windowHeight;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Rect rect;
  bool spacebar, moveLeftQ, moveRightE,
      moveDownN; // keys decide direction of bullet
};

Cannon *createCannon(SDL_Renderer *renderer, int windowWidth,
                     int windowHeight) {
  Cannon *c = malloc(sizeof(Cannon));
  if (!c)
    return NULL;

  c->windowWidth = windowWidth;
  c->windowHeight = windowHeight;
  c->renderer = renderer;
  c->dy = 100; // direction cannon shoots when press space at start of the game
  c->dx = 0;
  SDL_Surface *surface = IMG_Load("resources/ship_cannon.png");
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

  /*c->x = x - c->rect.w / 2;*/
  /*c->y = y - c->rect.h / 2;*/
  return c;
}

void drawCannon(Cannon *c) {
  SDL_RenderCopy(c->renderer, c->texture, NULL, &c->rect);
}

void destroyCannon(Cannon *c) {
  if (c) {
    if (c->texture)
      SDL_DestroyTexture(c->texture);
    free(c);
  }
}

void updateCannon(Cannon *pCannon, Ship *pShip) {
    // offset from where ship spawns to adjust where we
    // want to place cannon. change later to adjust for sprite sheet
    double cannonLocationX = 5;
    double cannonLocationY = 15;
  
    pCannon->rect.y = getShipY(pShip) + cannonLocationY;
    pCannon->rect.x = getShipX(pShip) + cannonLocationX;
  
    printf("Y coordinate: %d\n", pCannon->rect.y);
    printf("X coordinate: %d\n", pCannon->rect.x);
  }

void handleCannonEvent(Cannon *c, SDL_Event *event) {
  switch (event->key.keysym.scancode) {
  case SDL_SCANCODE_SPACE:
    printf("spacebar click registered\n");
    spawn_projectile(c->rect.x, c->rect.y, c->dx, c->dy); // fires projectile
    break;

  case SDL_SCANCODE_E:
    c->dy = 0;
    c->dx = 100;
    break;
  case SDL_SCANCODE_Q:
    c->dy = 0;
    c->dx = -100;
    break;
  case SDL_SCANCODE_N:
    c->dy = 100;
    c->dx = 0;
  default:
    break;
    // might need default statement
    // test
    break;
  }
}
void resetCannon(Cannon *c) {
  c->spacebar = false;
  c->moveDownN = false;
  c->moveLeftQ = false;
  c->moveRightE = false;
}