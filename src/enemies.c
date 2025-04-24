#include "enemies.h"
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
struct enemyImage{
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;    
};

struct enemy{
    float x, y, vx, vy;
    float health;
    float damage;
    bool active;
    int window_width,window_height,renderAngle;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect rect;
    SDL_Rect rectHitbox;
};

static void getStartValues(Enemy *name);

EnemyImage *initiateEnemy(SDL_Renderer *pRenderer)  
{
    static EnemyImage* pEnemyImage = NULL;
    if(pEnemyImage==NULL)
    {
        pEnemyImage = malloc(sizeof(struct enemyImage));
        SDL_Surface *surface = IMG_Load("resources/ufo.png");
        
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
    pEnemy->renderAngle = rand()%360;
    return pEnemy;
}

static void getStartValues(Enemy *pEnemy){
    srand(time(NULL));
    int startSpawnOnTheLeft =  rand() % 2; //0 or 1
    pEnemy->rectHitbox.x = pEnemy->rect.x + 10;
    pEnemy->rectHitbox.y = pEnemy->rect.y + 10;
    pEnemy->rectHitbox.w = pEnemy->rect.w - 20;
    pEnemy->rectHitbox.h = pEnemy->rect.h - 15;
    if (startSpawnOnTheLeft == 1)
    {
    pEnemy->x = pEnemy->window_width;
    pEnemy->y = rand() % (pEnemy->window_height - pEnemy->rect.h);

    float speed = 5.0f;
    pEnemy->vx = -speed; // rakt åt vänster
    pEnemy->vy = 0;      // ingen rörelse i y-led
    pEnemy->damage = 50;
    pEnemy->health = 100;
   
    }
    else 
    {
        pEnemy->x = 0; // also spawn at left
        pEnemy->y = rand() % (pEnemy->window_height - pEnemy->rect.h);

        float speed = 5.0f;
        pEnemy->vx = speed; // move right
        pEnemy->vy = 0;
    }
}

SDL_Rect getRectEnemy(Enemy *pEnemy){
    if (pEnemy->active == true)
    {
        return pEnemy->rectHitbox;
    }
}
void updateEnemy(Enemy *pEnemy){
    if(pEnemy->active == true)
    {
        pEnemy->x+=pEnemy->vx*0.1;
        pEnemy->y+=pEnemy->vy*0.1;
        if (pEnemy->x > pEnemy->window_width || pEnemy->x + pEnemy->rect.w < 0 ||
            pEnemy->y > pEnemy->window_height || pEnemy->y + pEnemy->rect.h < 0)
        {
            //getStartValues(pEnemy); //immediatly respawns enemy once it leaves windows
            pEnemy->active = false;
            return;
        }
        pEnemy->rect.x=pEnemy->x;
        pEnemy->rect.y=pEnemy->y;
        pEnemy->rectHitbox.x = pEnemy->rect.x + 10;
        pEnemy->rectHitbox.y = pEnemy->rect.y + 10;
   }
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

void damageEnemy(Enemy *pEnemy, int damage)
{
    pEnemy->health -= damage;
    printf("enemy health: %.2f\n", pEnemy->health);
}
bool checkIfActive(Enemy *pEnemy, int nrOfEnemies)
{
    if(pEnemy->active==true)
    {
        return true;
    }
    else
    {
        return false;
    }
}