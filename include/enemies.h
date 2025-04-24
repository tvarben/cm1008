#ifndef ENEMIES_H
#define ENEMIES_H
#include <SDL2/SDL.h>
#include <stdbool.h>


typedef struct enemyImage EnemyImage;
typedef struct enemy Enemy;

EnemyImage *initiateEnemy(SDL_Renderer *pRenderer);

Enemy *createEnemy(EnemyImage *pEnemyImage, int window_width, int window_height);
void updateEnemy(Enemy *pEnemy);
void drawEnemy(Enemy *pEnemy);
void destroyEnemy(Enemy *pEnemy);
SDL_Rect getRectEnemy(Enemy *pEnemy);
void destroyEnemyImage(EnemyImage *pEnemyImage);
void disableEnemy(Enemy *pEnemy);
void damageEnemy(Enemy *pEnemy, int damage);
bool checkIfActive(Enemy *pEnemy, int nrOfEnemies);


#endif