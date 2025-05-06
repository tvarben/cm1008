#include "../include/enemy3.h"
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
struct enemyImage3 {
  SDL_Renderer *pRenderer;
  SDL_Texture *pTexture;
};

struct enemy3 {
  float x, y, vx, vy;
  int health;
  int damage;
  bool active;
  int window_width, window_height, renderAngle;
  SDL_Renderer *pRenderer;
  SDL_Texture *pTexture;
  SDL_Rect rect;
  SDL_Rect rectHitbox;
};

static void getStartValues3(Enemy3 *name);

EnemyImage3 *initiateEnemy3(SDL_Renderer *pRenderer) {
  static EnemyImage3 *pEnemyImage3 = NULL;
  if (pEnemyImage3 == NULL) {
    pEnemyImage3 = malloc(sizeof(struct enemyImage3));
    SDL_Surface *surface = IMG_Load("resources/boss.png");

    if (!surface) {
      printf("Error: %s\n", SDL_GetError());
      return NULL;
    }

    pEnemyImage3->pRenderer = pRenderer;
    pEnemyImage3->pTexture = SDL_CreateTextureFromSurface(pRenderer, surface);
    SDL_FreeSurface(surface);

    if (!pEnemyImage3->pTexture) {
      printf("Error: %s\n", SDL_GetError());
      return NULL;
    }
  }
  return pEnemyImage3;
}

Enemy3 *createEnemy3(EnemyImage3 *pEnemyImage3, int window_width, int window_height) {
  Enemy3 *pEnemy3 = malloc(sizeof(struct enemy3));
  pEnemy3->pRenderer = pEnemyImage3->pRenderer;
  pEnemy3->pTexture = pEnemyImage3->pTexture;
  pEnemy3->window_width = window_width;
  pEnemy3->window_height = window_height;
  SDL_QueryTexture(pEnemyImage3->pTexture, NULL, NULL, &(pEnemy3->rect.w),&(pEnemy3->rect.h));
  getStartValues3(pEnemy3);
  pEnemy3->active = true;
  return pEnemy3;
}

static void getStartValues3(Enemy3 *pEnemy3) {
  int startSpawnOnTheLeft = rand() % 2; // 0 or 1
  pEnemy3->rectHitbox.x = pEnemy3->rect.x + 10;
  pEnemy3->rectHitbox.y = pEnemy3->rect.y + 10;
  pEnemy3->rectHitbox.w = pEnemy3->rect.w - 50;
  pEnemy3->rectHitbox.h = pEnemy3->rect.h - 50;
  pEnemy3->damage = 2;
  pEnemy3->health = 100;   // changed health
  float speed = 10;
  if (startSpawnOnTheLeft == 1) {
    pEnemy3->x = pEnemy3->window_width - pEnemy3->rect.w; // spawn at right
    pEnemy3->y = pEnemy3->window_height / 2; //spawn at middle
    pEnemy3->vx = -speed; // rakt åt vänster
    pEnemy3->vy = -speed; // move up
  } else {
      pEnemy3->x = 0; // also spawn at left
      pEnemy3->y = pEnemy3->window_height / 2; //spawn at middle
      pEnemy3->vx = speed; // move right
      pEnemy3->vy = speed; // move down
  }
}

SDL_Rect getRectEnemy3(Enemy3 *pEnemy3) {
  if (pEnemy3->active == true) {
    return pEnemy3->rectHitbox;
  }
  SDL_Rect empty = {0, 0, 0, 0};
  return empty;
}
void updateEnemy3(Enemy3 *pEnemy3) {
  if (pEnemy3->active == true)
  {
    pEnemy3->x+=pEnemy3->vx*0.1;
    pEnemy3->y+=pEnemy3->vy*0.1;
    if ((pEnemy3->x + pEnemy3->rect.w -10) > pEnemy3->window_width || pEnemy3->x + 10 < 0)
    {
        pEnemy3->vx = pEnemy3->vx * -1; // reverse direction
        return;
    }
    if ((pEnemy3->y + pEnemy3->rect.h -10) > pEnemy3->window_height || pEnemy3->y + 10 < 0)
    {
        pEnemy3->vy = pEnemy3->vy * -1; // reverse direction
        return;
    }
    pEnemy3->rect.x = pEnemy3->x;
    pEnemy3->rect.y = pEnemy3->y;
    pEnemy3->rectHitbox.x = pEnemy3->rect.x + 40;
    pEnemy3->rectHitbox.y = pEnemy3->rect.y + 20;
  }
}

void drawEnemy3(Enemy3 *pEnemy3) {
  if (pEnemy3->active == true) {
    SDL_RenderCopyEx(pEnemy3->pRenderer, pEnemy3->pTexture, NULL, &(pEnemy3->rect), 0, NULL,SDL_FLIP_NONE); // made 0 to not rotate enemy.png
  }
}

void destroyEnemy3(Enemy3 *pEnemy3) { free(pEnemy3); }

void destroyEnemyImage3(EnemyImage3 *pEnemyImage3) {
  SDL_DestroyTexture(pEnemyImage3->pTexture);
}

void disableEnemy3(Enemy3 *pEnemy3) {
  if (pEnemy3->health <= 0) {
    pEnemy3->active = false;
  }
}

void damageEnemy3(Enemy3 *pEnemy3, int damage, int i) {
  pEnemy3->health -= damage;
  if (pEnemy3->health <= 0 && pEnemy3->active == true) {
    pEnemy3->active = false;
    printf("Enemy nr %d dead\n The boss died\n", i);
  }
  if (pEnemy3->health < 0) pEnemy3->health = 0;
}

// bool isInWindow3(Enemy3 *pEnemy3) {
//   if (pEnemy3->x > pEnemy3->window_width || pEnemy3->x + pEnemy3->rect.w < 0 || pEnemy3->y > pEnemy3->window_height || pEnemy3->y + pEnemy3->rect.h < 0) 
//   {
//     pEnemy3->vx = pEnemy3->vx * -1; // reverse direction
//     pEnemy3->vy = pEnemy3->vy * -1; // reverse direction
//     return false;
//   } 
//   else {
//     return true;
//   }
// }

bool isEnemy3Active(Enemy3 *pEnemy3) {
  if (pEnemy3->active == false) {
    return false;
  } else {
    return true;
  }
}
void printEnemy3Health(Enemy3 *pEnemy3) {
  if (pEnemy3->active == true) {
    printf("Health: %d\n", pEnemy3->health);
  }
}