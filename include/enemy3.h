#ifndef ENEMIES3_H
#define ENEMIES3_H
#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct enemyImage3 EnemyImage3;
typedef struct enemy3 Enemy3;

EnemyImage3 *initiateEnemy3(SDL_Renderer *pRenderer);

Enemy3 *createEnemy3(EnemyImage3 *pEnemyImage, int window_width,
                     int window_height);
void updateEnemy3(Enemy3 *pEnemy);
void drawEnemy3(Enemy3 *pEnemy);
void destroyEnemy3(Enemy3 *pEnemy);
SDL_Rect getRectEnemy3(Enemy3 *pEnemy);
void destroyEnemyImage3(EnemyImage3 *pEnemyImage);
void disableEnemy3(Enemy3 *pEnemy);
void damageEnemy3(Enemy3 *pEnemy, int damage, int i);
//bool isInWindow3(Enemy3 *pEnemy);
bool isEnemy3Active(Enemy3 *pEnemy);
void printEnemy3Health(Enemy3 *pEnemy);

#endif