#include "../include/bullet.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <stdbool.h>
#include <stdio.h>
#define MAX_PROJECTILES 100
bullet projectiles[MAX_PROJECTILES]; // 100 bullets can be active at the time
#include <stdlib.h>
#define projectile_width 8
#define projectile_length 4
#define WINDOW_WIDTH 1160
#define WINDOW_HEIGHT 700

void spawn_projectile(float x, float y, float dx, float dy) {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (!projectiles[i].active) {
      projectiles[i].x = x;
      projectiles[i].y = y;
      projectiles[i].vx = dx;
      projectiles[i].vy = dy;
      projectiles[i].active = 1;
      projectiles[i].rect.w = projectile_width; // size of projectile
      projectiles[i].rect.h = projectile_length;
      printf("projectile shot \n");
      return;
    }
  }
}

void update_projectiles(float delta_time) {
  //
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (projectiles[i].active) {
      projectiles[i].x += projectiles[i].vx * delta_time;
      projectiles[i].y += projectiles[i].vy * delta_time;

      // Deactivate if off screen (based on wndiow size)
      if (projectiles[i].x < 0 || projectiles[i].x > WINDOW_WIDTH ||
          projectiles[i].y < 0 || projectiles[i].y > WINDOW_HEIGHT) {
        projectiles[i].active = 0;
      }

      // Update rect position for rendering
      projectiles[i].rect.x = (int)projectiles[i].x;
      projectiles[i].rect.y = (int)projectiles[i].y;
    } // casting int gets rid of compiler warnings
  }
}

void render_projectiles(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow

  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (projectiles[i].active) {
      SDL_RenderFillRect(renderer, &projectiles[i].rect);
    }
  }
}