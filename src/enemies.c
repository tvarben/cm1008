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
    int window_width,window_height,renderAngle;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect rect;
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
    pEnemy->renderAngle = rand()%360;
    return pEnemy;
}

static void getStartValues(Enemy *pEnemy){
    srand(time(NULL));
    int startSpawnOnTheLeft =  rand() % 2; //0 or 1

    if (startSpawnOnTheLeft == 1)
    {
    pEnemy->x = pEnemy->window_width;
    pEnemy->y = rand() % (pEnemy->window_height - pEnemy->rect.h);

    float speed = 5.0f;
    pEnemy->vx = -speed; // rakt åt vänster
    pEnemy->vy = 0;      // ingen rörelse i y-led

    pEnemy->rect.x = pEnemy->x;
    pEnemy->rect.y = pEnemy->y;
    }
    else 
    {
        pEnemy->x = 0; // also spawn at left
        pEnemy->y = rand() % (pEnemy->window_height - pEnemy->rect.h);

        float speed = 5.0f;
        pEnemy->vx = speed; // move right
        pEnemy->vy = 0;

        pEnemy->rect.x = pEnemy->x;
        pEnemy->rect.y = pEnemy->y;
    }
}

SDL_Rect getRectEnemy(Enemy *pEnemy){
    return pEnemy->rect;
}
void updateEnemy(Enemy *pEnemy){
    pEnemy->x+=pEnemy->vx*0.1;
    pEnemy->y+=pEnemy->vy*0.1;
    if (pEnemy->x>pEnemy->window_width||pEnemy->y>pEnemy->window_height){
        getStartValues(pEnemy);
        return;
    }
    pEnemy->rect.x=pEnemy->x;
    pEnemy->rect.y=pEnemy->y;
}

void drawEnemy(Enemy *pEnemy){
    SDL_RenderCopyEx(pEnemy->pRenderer, pEnemy->pTexture, NULL, &(pEnemy->rect), 0, NULL, SDL_FLIP_NONE); //made 0 to not rotate enemy.png
}

void destroyEnemy(Enemy *pEnemy){
    free(pEnemy);
}

void destroyEnemyImage(EnemyImage *pEnemyImage){
    SDL_DestroyTexture(pEnemyImage->pTexture);
}
