#ifndef ENEMY_2_H
#define ENEMY_2_H
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "data.h"

typedef struct enemyImage_2 EnemyImage_2;
typedef struct enemy2 Enemy_2;

EnemyImage_2 *initiateEnemy_2(SDL_Renderer *pRenderer);
Enemy_2 *createEnemy_2(EnemyImage_2 *pEnemyImage, int window_width, int window_height);
void updateEnemy_2(Enemy_2 *pEnemy);
void drawEnemy_2(Enemy_2 *pEnemy);
void destroyEnemy_2(Enemy_2 *pEnemy);
SDL_Rect getRectEnemy_2(Enemy_2 *pEnemy);
void destroyEnemyImage_2(EnemyImage_2 *pEnemyImage);
void disableEnemy_2(Enemy_2 *pEnemy);
void damageEnemy_2(Enemy_2 *pEnemy, int damage, int i);
bool isInWindow_2(Enemy_2 *pEnemy);
bool isEnemy_2Active(Enemy_2 *pEnemy);
void printEnemy_2Health(Enemy_2 *pEnemy);

Enemy_2 *createEnemy_2_OnClients(EnemyImage_2 *pEnemyImage, int window_width, int window_height, Enemy_2_Data enemyData);
void updateEnemy_2_OnClients(Enemy_2 *pEnemy, Enemy_2_Data enemyData);
void getEnemy_2_DataPackage(Enemy_2 *pEnemy, Enemy_2_Data *pEnemyData);

#endif 