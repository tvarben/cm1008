#include "enemy_1.h"
#include "ship_data.h"
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define ENEMY_1_PATH "../lib/resources/ufo.png"
#define INITIALSPEED 3;
struct enemyImage{
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;    
};

struct enemy{
    float x, y, vx, vy;
    int health;
    int damage;
    bool active;
    int window_width,window_height;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect rect;
    SDL_Rect rectHitbox;
};
static void getStartValues(Enemy *pEnemy);

EnemyImage *initiateEnemy(SDL_Renderer *pRenderer)  
{
    static EnemyImage* pEnemyImage = NULL;
    if(pEnemyImage==NULL)
    {
        pEnemyImage = malloc(sizeof(struct enemyImage));
        SDL_Surface *surface = IMG_Load(ENEMY_1_PATH);
        
        if(!surface)
        {
            printf("Error: %s\n",SDL_GetError());
            return NULL;
        }

        pEnemyImage->pRenderer = pRenderer;
        pEnemyImage->pTexture = SDL_CreateTextureFromSurface(pRenderer, surface);
        SDL_FreeSurface(surface);

        if(!pEnemyImage->pTexture)
        {
            printf("Error: %s\n",SDL_GetError());
            return NULL;
        }
    }
    return pEnemyImage;
}

Enemy *createEnemy(EnemyImage *pEnemyImage, int window_width, int window_height){
    Enemy *pEnemy = malloc(sizeof(struct enemy));
    pEnemy->pRenderer = pEnemyImage->pRenderer;
    pEnemy->pTexture = pEnemyImage->pTexture;
    pEnemy->window_width = window_width;
    pEnemy->window_height = window_height;
    SDL_QueryTexture(pEnemyImage->pTexture,NULL,NULL,&(pEnemy->rect.w),&(pEnemy->rect.h));
    getStartValues(pEnemy);
    pEnemy->active = true;
    return pEnemy;
}

Enemy *createEnemyOnClient(EnemyImage *pEnemyImage, int window_width, int window_height, Enemy_1_Data enemyData) {
    Enemy *pEnemy = malloc(sizeof(struct enemy));
    pEnemy->pRenderer = pEnemyImage->pRenderer;
    pEnemy->pTexture = pEnemyImage->pTexture;
    pEnemy->window_width = window_width;
    pEnemy->window_height = window_height;
    SDL_QueryTexture(pEnemyImage->pTexture,NULL,NULL,&(pEnemy->rect.w),&(pEnemy->rect.h));
    getStartValuesFromServer(pEnemy, enemyData);
    return pEnemy;
}

static void getStartValuesFromServer(Enemy *pEnemy, Enemy_1_Data enemyData) {
    pEnemy->rectHitbox.x = pEnemy->rect.x + 10;
    pEnemy->rectHitbox.y = pEnemy->rect.y + 10;
    pEnemy->rectHitbox.w = pEnemy->rect.w - 20;
    pEnemy->rectHitbox.h = pEnemy->rect.h - 10;
    pEnemy->damage = 1;
    pEnemy->health = 2;
    pEnemy->x = enemyData.x;
    pEnemy->y = enemyData.y;
    pEnemy->active = enemyData.active;
}

static void getStartValues(Enemy *pEnemy){
    int startSpawnOnTheLeft =  rand() % 2; //0 or 1
    pEnemy->rectHitbox.x = pEnemy->rect.x + 10;
    pEnemy->rectHitbox.y = pEnemy->rect.y + 10;
    pEnemy->rectHitbox.w = pEnemy->rect.w - 20;
    pEnemy->rectHitbox.h = pEnemy->rect.h - 10;
    pEnemy->damage = 1;
    pEnemy->health = 2;
    //float speed = rand() % (50 - 15 + 1) + 15;
    float speed = INITIALSPEED;
    if (startSpawnOnTheLeft)
    {
        pEnemy->x = pEnemy->window_width;
        pEnemy->y = rand() % (pEnemy->window_height - pEnemy->rect.h);
        pEnemy->vx = -speed; // rakt åt vänster
        pEnemy->vy = 0;      // ingen rörelse i y-led
   
    }
    else 
    {
        pEnemy->x = 0; // also spawn at left
        pEnemy->y = rand() % (pEnemy->window_height - pEnemy->rect.h);

        pEnemy->vx = speed; // move right
        pEnemy->vy = 0;
    }
}

SDL_Rect getRectEnemy(Enemy *pEnemy)
{
    if (pEnemy->active == true)
    {
        return pEnemy->rectHitbox;
    }
    SDL_Rect empty = {0,0,0,0};
    return empty;
}
//void updateEnemy(Enemy *pEnemy, Uint32 deltaTime){ //why deltaTime? if needed create it inside enemy_1.c beacause deltaTime we use in clientMain.c is messing up the enemy speed!
void updateEnemy(Enemy *pEnemy){
    if(pEnemy->active == true)
    {
        pEnemy->x += pEnemy->vx;
        pEnemy->y += pEnemy->vy;
        if (pEnemy->x > pEnemy->window_width || pEnemy->x + pEnemy->rect.w < 0 ||
            pEnemy->y > pEnemy->window_height || pEnemy->y + pEnemy->rect.h < 0)
        {
            //getStartValues(pEnemy); //immediatly respawns enemy once it leaves windows
            pEnemy->active = false;
            printf("Enemy has fled \n");
            return;
        }
        pEnemy->rect.x = pEnemy->x;
        pEnemy->rect.y = pEnemy->y;
        pEnemy->rectHitbox.x = pEnemy->rect.x + 10;
        pEnemy->rectHitbox.y = pEnemy->rect.y + 10;
   }
}

void updateEnemyOnClients(Enemy *pEnemy, Enemy_1_Data enemyData) {
    pEnemy->x = enemyData.x;
    pEnemy->y = enemyData.y;
    pEnemy->active = enemyData.active;
    pEnemy->rect.x = pEnemy->x;
    pEnemy->rect.y = pEnemy->y;
    pEnemy->rectHitbox.x = pEnemy->rect.x + 10;
    pEnemy->rectHitbox.y = pEnemy->rect.y + 10;
}

void drawEnemy(Enemy *pEnemy){
    if (pEnemy->active == true)
    {
        SDL_RenderCopyEx(pEnemy->pRenderer, pEnemy->pTexture, NULL, &(pEnemy->rect), 0, NULL, SDL_FLIP_NONE); //made 0 to not rotate enemy.png
    }
}

void destroyEnemy(Enemy *pEnemy){
        free(pEnemy);
}

void destroyEnemyImage(EnemyImage *pEnemyImage){
    SDL_DestroyTexture(pEnemyImage->pTexture);
}

void disableEnemy(Enemy *pEnemy)
{
    if(pEnemy->health <= 0)
    {
    pEnemy->active = false;
    }
}

void damageEnemy(Enemy *pEnemy, int damage, int i)
{
    pEnemy->health -= damage;
    if(pEnemy->health <= 0 && pEnemy->active == true)
    {
    pEnemy->active = false;
    printf("Enemy nr %d dead\n", i);
    }
    if (pEnemy->health < 0) pEnemy->health = 0;
}

bool isInWindow(Enemy *pEnemy)
{
    if (pEnemy->x > pEnemy->window_width || pEnemy->x + pEnemy->rect.w < 0 ||
        pEnemy->y > pEnemy->window_height || pEnemy->y + pEnemy->rect.h < 0)
    {
        disableEnemy(pEnemy);  
        return false;
    }
    else
    {
        return true;
    }
}

bool isEnemyActive(Enemy *pEnemy)
{
    if (pEnemy->active == false)
    {
        return false;
    }
    else
    {
        return true;
    }
    
}
void printEnemyHealth(Enemy *pEnemy)
{
    if(pEnemy->active == true)
    {
        printf("Health: %d\n",pEnemy->health);
    }
}

void getEnemy_1_DataPackage(Enemy *pEnemy, Enemy_1_Data *pEnemyData) {
    pEnemyData->x = pEnemy->x;
    pEnemyData->y = pEnemy->y;
    pEnemyData->active = pEnemy->active;
    // printf("Sending Enemy data to data package:\n Enemies_1 Enemy.active: %d, Enemy.x, Enemy.y: [%.2f,%.2f]\n", pEnemy->active, pEnemy->x, pEnemy->y);
    // printf("Sending Enemy data to data package:\n Enemies_1 Data.Enemy.active: %d, Data.Enemy.x, Data.Enemy.y: [%.2f,%.2f]\n", pEnemyData->active, pEnemyData->x, pEnemyData->y);
}

void updateEnemies_1_WithServerData(Enemy *pEnemy, Enemy_1_Data *pEnemyData) {
    printf("Copying relevant enemy data...\n");
    pEnemy->x = pEnemyData->x;
    printf("x coordinate data copied...\n");
    pEnemy->y = pEnemyData->y;
    printf("y coordinate data copied...\n");
    pEnemy->active = pEnemyData->active;
    printf("Success!\n");
}


