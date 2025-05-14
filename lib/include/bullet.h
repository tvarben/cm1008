#ifndef BULLET_H
#define BULLET_H
#include "ship.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#define MAX_PROJECTILES 100

typedef struct bullet Bullet;

struct bullet {
  float x, y, vx, vy;
  bool active;
  SDL_Rect rect;
};

void spawn_projectile(float x, float y, float dx, float dy);
void getProjectileRects(SDL_Rect rectArray[]);
void update_projectiles(Uint32 delta_time);
void render_projectiles(SDL_Renderer *renderer);
SDL_Rect getRectProjectile(Bullet projectile[]);
void resetAllBullets();
void removeProjectile(int i);
#endif