#ifndef ENEMIES3_H
#define ENEMIES3_H
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "ship_data.h"

typedef struct enemyImage_3 EnemyImage_3;
typedef struct enemy3 Enemy_3;

EnemyImage_3 *initiateEnemy_3(SDL_Renderer *pRenderer);

Enemy_3 *createEnemy_3(EnemyImage_3 *pEnemyImage, int window_width, int window_height);
void updateEnemy_3(Enemy_3 *pEnemy);
void drawEnemy_3(Enemy_3 *pEnemy);
void destroyEnemy_3(Enemy_3 *pEnemy);
SDL_Rect getRectEnemy_3(Enemy_3 *pEnemy);
void destroyEnemyImage_3(EnemyImage_3 *pEnemyImage);
void disableEnemy_3(Enemy_3 *pEnemy);
void damageEnemy_3(Enemy_3 *pEnemy, int damage, int i);
//bool isInWindow_3(Enemy_3 *pEnemy);
bool isEnemy_3Active(Enemy_3 *pEnemy);
void printEnemy_3Health(Enemy_3 *pEnemy);

#endif