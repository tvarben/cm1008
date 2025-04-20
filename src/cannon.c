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
  bool lastFacedLeft;
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
  c->dy = 0; // direction cannon shoots when press space at start of the game
  c->dx = 100;
  c->lastFacedLeft = false;
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
  SDL_RendererFlip flip = c->lastFacedLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
  SDL_RenderCopyEx(c->renderer, c->texture, NULL, &c->rect, 0, NULL, flip);
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
    double cannonLocationX = 28;
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
    if (c->lastFacedLeft == true)
    {
      spawn_projectile(c->rect.x-8, c->rect.y+15, c->dx, c->dy); // fires projectile
    }
    else 
    {
      spawn_projectile(c->rect.x+20, c->rect.y+15, c->dx, c->dy); // fires projectile
    }
    break;
  case SDL_SCANCODE_D: case SDL_SCANCODE_RIGHT:
    c->dy = 0;
    c->dx = 100;
    c->lastFacedLeft = false;
    break;
  case SDL_SCANCODE_A: case SDL_SCANCODE_LEFT:
    c->dy = 0;
    c->dx = -100;
    c->lastFacedLeft = true;
    break;
  default:
    if (c->lastFacedLeft == true)
    {
      c->dy = 0;
      c->dx = -100;
    }
    else
    {
      c->dy = 0;
      c->dx = 100;
    }
    break;
  }
}
void resetCannon(Cannon *c) {
  c->spacebar = false;
  c->moveDownN = false;
  c->moveLeftQ = false;
  c->moveRightE = false;
}