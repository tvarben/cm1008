#include "../include/cannon.h"
#include "../include/bullet.h"
#include "ship.h"
#include "sound.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct Cannon {
  int dy, dx;
  int windowWidth, windowHeight;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Rect rect;
  bool lastFacedLeft;
  bool spacebar, up, down, left, right, ableToShoot;
  CannonDirection direction;
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
  SDL_RendererFlip flip =
      c->lastFacedLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
  SDL_RenderCopyEx(c->renderer, c->texture, NULL, &c->rect, 0, NULL, flip);
}

void destroyCannon(Cannon *c) {
  if (c) {
    if (c->texture)
      SDL_DestroyTexture(c->texture);
    free(c);
  }
}

void updateCannon(Cannon *c, Ship *s) {
  // double cannonLocationX = 28;
  // double cannonLocationY = 15;
  int speed = 1000;
  SDL_Rect shipRect = getShipRect(s);
  Mix_Chunk *pBulletSFX;

  c->rect.y = getShipY(s) + (shipRect.h / 2); // + cannonLocationY;
  c->rect.x = getShipX(s) + (shipRect.w / 2); // + cannonLocationX;

  if (!c->spacebar || !c->ableToShoot) return;
  switch(c->direction) {
    case UP:
      spawn_projectile(c->rect.x, c->rect.y, 0, -speed);
      break;
    case UP_L:
      spawn_projectile(c->rect.x, c->rect.y, -speed/sqrt(2), -speed/sqrt(2));
      break;
    case UP_R:
      spawn_projectile(c->rect.x, c->rect.y, speed/sqrt(2), -speed/sqrt(2));
      break;
    
    case DOWN:
      spawn_projectile(c->rect.x, c->rect.y, 0, speed);
      break;
    case DOWN_L:
      spawn_projectile(c->rect.x, c->rect.y, -speed/sqrt(2), speed/sqrt(2));
      break;
    case DOWN_R:
      spawn_projectile(c->rect.x, c->rect.y, speed/sqrt(2), speed/sqrt(2));
      break;
    case LEFT:
      spawn_projectile(c->rect.x, c->rect.y, -speed, 0);
      break;
    case RIGHT:
      spawn_projectile(c->rect.x, c->rect.y, speed, 0);
      break;
  }
  playSound(&pBulletSFX, "resources/pew.wav",-1);
  c->ableToShoot = false;

  // if (isLeft(pShip)) {
  //   pCannon->lastFacedLeft = true;
  //   pCannon->dx = -100;
  //   pCannon->dy = 0;
  // } else {
  //   pCannon->lastFacedLeft = false;
  //   pCannon->dx = 100;
  //   pCannon->dy = 0;
  // }
}

void handleCannonEvent(Cannon *c, SDL_Event *event) {
  switch(event->type) {
    case SDL_KEYDOWN:
      switch(event->key.keysym.scancode) {
        case SDL_SCANCODE_UP: c->up = true; break;
        case SDL_SCANCODE_LEFT: c->left = true; break;
        case SDL_SCANCODE_DOWN: c->down = true; break;
        case SDL_SCANCODE_RIGHT: c->right = true; break;
        case SDL_SCANCODE_SPACE: c->spacebar = true; break;
      }
    break;
    case SDL_KEYUP:
    switch(event->key.keysym.scancode) {
      case SDL_SCANCODE_UP: c->up = false; printf("-UP\n"); break;
      case SDL_SCANCODE_LEFT: c->left = false; printf("-Left\n"); break;
      case SDL_SCANCODE_DOWN: c->down = false; printf("-down\n"); break;
      case SDL_SCANCODE_RIGHT: c->right = false; printf("-right\n"); break;
      case SDL_SCANCODE_SPACE: 
        c->spacebar = false;
        c->ableToShoot = true; 
        break;
    }
    break;
  }

  if (c->up && !c->down && !c->right && !c->left) c->direction = UP;
  else if (c->down && !c->up && !c->right && !c->left) c->direction = DOWN;
  else if (c->left && !c->right && !c->up && !c->down) c->direction = LEFT;
  else if (c->right && !c->left && !c->up && !c->down) c->direction = RIGHT;
  else if (c->up && c->right && !c->left && !c->down) c->direction = UP_R;
  else if (c->up && c->left && !c->right && !c->down) c->direction = UP_L;
  else if (c->down && c->right && !c->left && !c->up) c->direction = DOWN_R;
  else if (c->down && c->left && !c->right && !c->up) c->direction = DOWN_L;

  // if (event->type == SDL_KEYDOWN && event->key.keysym.scancode == SDL_SCANCODE_SPACE) 
  //   if (c->lastFacedLeft) {
  //     spawn_projectile(c->rect.x - 8, c->rect.y, -1000, 0);
  //   } else {
  //     spawn_projectile(c->rect.x + 20, c->rect.y, 1000, 0);
  //   }
  //   Mix_Chunk *pBulletSFX;
  //    playSound(&pBulletSFX,"resources/pew.wav",-1);
  
 }

void resetCannon(Cannon *c) {
  c->spacebar = false;
  c->up = false;
  c->down = false;
  c->left = false;
  c->right = false;
  c->ableToShoot = true;
}
