#ifndef ENEMY_1_H
#define ENEMY_1_H
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "ship_data.h"


typedef struct enemyImage EnemyImage;
typedef struct enemy Enemy;

EnemyImage *initiateEnemy(SDL_Renderer *pRenderer);

Enemy *createEnemy(EnemyImage *pEnemyImage, int window_width, int window_height);
//void updateEnemy(Enemy *pEnemy, Uint32 deltaTime);
void updateEnemy(Enemy *pEnemy);
void drawEnemy(Enemy *pEnemy);
void destroyEnemy_1(Enemy *pEnemy);
SDL_Rect getRectEnemy(Enemy *pEnemy);
void destroyEnemy_1Image(EnemyImage *pEnemyImage);
void disableEnemy(Enemy *pEnemy);
void damageEnemy(Enemy *pEnemy, int damage, int i);
bool isInWindow(Enemy *pEnemy);
bool isEnemyActive(Enemy *pEnemy);
void printEnemyHealth(Enemy *pEnemy);
void updateEnemyOnClients(Enemy *pEnemy, Enemy_1_Data enemyData);
Enemy *createEnemyOnClient(EnemyImage *pEnemyImage, int window_width, int window_height, Enemy_1_Data enemyData);
static void getStartValuesFromServer(Enemy *pEnemy, Enemy_1_Data enemyData);

void getEnemy_1_DataPackage(Enemy* pEnemy, Enemy_1_Data* pEnemyData);


#endif