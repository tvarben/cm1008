#include "../include/cannon.h"
#include "bullet.c"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct Cannon {
  /*float x, y;*/
  int windowWidth, windowHeight;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Rect rect;
  bool spacebar;
};

Cannon *createCannon(int x, int y, SDL_Renderer *renderer, int windowWidth,
                     int windowHeight) {
  Cannon *c = malloc(sizeof(Cannon));
  if (!c)
    return NULL;

  c->windowWidth = windowWidth;
  c->windowHeight = windowHeight;
  c->renderer = renderer;

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
  double cannonLocationX = 5;
  // offset from where ship spawns to adjust where we
  // want to place cannon. change later to adjust for sprite sheet
  double cannonLocationY = 15;
  /*pCannon->y = getShipY(pShip); //*/
  /*pCannon->x = getShipX(pShip);*/
  pCannon->rect.y = getShipY(pShip) + cannonLocationY;
  pCannon->rect.x = getShipX(pShip) + cannonLocationX;
  /*printf("Y coordinate: %d\n", pCannon->rect.y);*/
  /*printf("X coordinate: %d\n", pCannon->rect.x);*/
}

void handleCannonEvent(Cannon *c, SDL_Event *event) {
  bool down = event->type == SDL_KEYDOWN;
  switch (event->key.keysym.scancode) {
  case SDL_SCANCODE_SPACE:
    c->spacebar = down;
    printf("spacebar click registered\n");
    break;
  default:
    break;
  }
}
void resetCannon(Cannon *c) { c->spacebar = false; }
