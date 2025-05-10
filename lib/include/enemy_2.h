#ifndef ENEMY_2_H
#define ENEMY_2_H
#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct enemyImage2 EnemyImage2;
typedef struct enemy2 Enemy2;

EnemyImage2 *initiateEnemy2(SDL_Renderer *pRenderer);

Enemy2 *createEnemy2(EnemyImage2 *pEnemyImage, int window_width,
                     int window_height);
void updateEnemy2(Enemy2 *pEnemy);
void drawEnemy2(Enemy2 *pEnemy);
void destroyEnemy2(Enemy2 *pEnemy);
SDL_Rect getRectEnemy2(Enemy2 *pEnemy);
void destroyEnemyImage2(EnemyImage2 *pEnemyImage);
void disableEnemy2(Enemy2 *pEnemy);
void damageEnemy2(Enemy2 *pEnemy, int damage, int i);
bool isInWindow2(Enemy2 *pEnemy);
bool isEnemy2Active(Enemy2 *pEnemy);
void printEnemy2Health(Enemy2 *pEnemy);

#endif