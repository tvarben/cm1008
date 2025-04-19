#ifndef BULLET_H
#define BULLET_H
#include "ship.h"
#include <SDL2/SDL.h>

typedef struct bullet bullet;
struct bullet {
  float x, y, vx, vy;
  int active;
  SDL_Rect rect;
};

void spawn_projectile(float x, float y, float dx, float dy);

void update_projectiles(float delta_time);

void render_projectiles(SDL_Renderer *renderer);
#endif