#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
struct bullet {
  float x, y, vx, vy;
  int time, window_width, window_height;
};
typedef struct bullet Bullet;

Bullet *createBullet(int window_width, int window_height) {
  Bullet *pBullet = malloc(sizeof(Bullet));
  pBullet->window_width = window_width;
  pBullet->window_height = window_height;
  pBullet->time = 0;

  return pBullet;
}

void updateBullet(Bullet *pBullet) {
  if (pBullet->time == 0)
    return;
  pBullet->x += pBullet->vx;
  pBullet->y += pBullet->vy;
  if (pBullet->x < 0)
    pBullet->x += pBullet->window_width;
  else if (pBullet->x > pBullet->window_width)
    pBullet->x -= pBullet->window_width;
  if (pBullet->y < 0)
    pBullet->y += pBullet->window_height;
  else if (pBullet->y > pBullet->window_height)
    pBullet->y -= pBullet->window_height;
  (pBullet->time)--;
  return;
}
void startBullet(Bullet *pBullet, float x, float y, float vx, float vy) {
  pBullet->x = x;
  pBullet->y = y;
  pBullet->vx = vx;
  pBullet->vy = vy;
  pBullet->time = 300;
}
void killBullet(Bullet *pBullet) { pBullet->time = 0; }

void drawBullet(Bullet *pBullet, SDL_Renderer *pRenderer) {
  if (pBullet->time == 0)
    return;
  SDL_RenderDrawPoint(pRenderer, pBullet->x, pBullet->y);
  SDL_RenderDrawPoint(pRenderer, pBullet->x + 1, pBullet->y);
  SDL_RenderDrawPoint(pRenderer, pBullet->x, pBullet->y + 1);
  SDL_RenderDrawPoint(pRenderer, pBullet->x + 1, pBullet->y + 1);
}

float xBullet(Bullet *pBullet) { return pBullet->x; }

float yBullet(Bullet *pBullet) { return pBullet->y; }

void destroyBullet(Bullet *pBullet) { free(pBullet); }

int aliveBullet(Bullet *pBullet) {
  return pBullet->time > 0;
}

/*void getBulletSendData(Bullet *pBullet,BulletData *pBulletData);*/
/*void updateBulletWithRecievedData(Bullet *pBullet,BulletData *pBulletData)*/;
