#include "cannon.h"
#include "bullet.h"
#include "ship.h"
#include "ship_data.h"
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
  bool spacebar, moveLeftQ, moveRightE, moveDownN, shoot; // keys decide direction of bullet
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
  c->dx = 400;
  c->lastFacedLeft = false;
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
  double cannonLocationX = 28;
  double cannonLocationY = 15;

  pCannon->rect.y = getShipY(pShip) + cannonLocationY;
  pCannon->rect.x = getShipX(pShip) + cannonLocationX;

  if (isLeft(pShip)) {
    pCannon->lastFacedLeft = true;
    pCannon->dx = -100;
    pCannon->dy = 0;
  } else {
    pCannon->lastFacedLeft = false;
    pCannon->dx = 100;
    pCannon->dy = 0;
  }
}

void handleCannonEvent(Cannon *cannon, ClientCommand command) {
  //if (command == SHOOT) {
    if (cannon->lastFacedLeft) {
        spawn_projectile(cannon->rect.x - 8, cannon->rect.y + 15, -5, 0);
        //printf("left\n");
    } else {
        spawn_projectile(cannon->rect.x + 20, cannon->rect.y + 15, 5, 0);
        //printf("right\n");
    }
    //command = STOP_SHOOT;
  //}
}

void applyCannonCommand(Cannon *pCannon, ClientCommand command) {
  switch (command) {
      case SHOOT:
          pCannon->shoot = true;
          break;
      case STOP_SHOOT:
          pCannon->shoot = false;
          break;
      case QUIT:
      default:
          break;
  }
}    

void resetCannon(Cannon *c) {
  c->spacebar = false;
  c->moveDownN = false;
  c->moveLeftQ = false;
  c->moveRightE = false;
}