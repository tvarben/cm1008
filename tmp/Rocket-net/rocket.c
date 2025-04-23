#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include "rocket_data.h"
#include "rocket.h"
#include "bullet.h"
#define SQUAREOFMAXSPEED 30

struct rocket{
    float x, y, vx, vy, xStart, yStart;
    int angle, alive;
    int window_width,window_height;
    Bullet *pBullet;
    SDL_Renderer *pRenderer;    
    SDL_Texture *pTexture;
    SDL_Rect shipRect;
};

float distance(int x1, int y1, int x2, int y2);

Rocket *createRocket(int number, SDL_Renderer *pRenderer, int window_width, int window_height){
    Rocket *pRocket = malloc(sizeof(struct rocket));
    pRocket->vx=pRocket->vy=0;
    pRocket->angle=0;
    pRocket->alive=1;
    pRocket->window_width = window_width;
    pRocket->window_height = window_height;
    pRocket->pBullet = createBullet(window_width,window_height);
    SDL_Surface *pSurface = IMG_Load("../lib/resources/Ship.png");
    if(!pSurface){
        printf("Error: %s\n",SDL_GetError());
        return NULL;
    }
    pRocket->pRenderer = pRenderer;
    pRocket->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if(!pRocket->pTexture){
        printf("Error: %s\n",SDL_GetError());
        return NULL;
    }
    SDL_QueryTexture(pRocket->pTexture,NULL,NULL,&(pRocket->shipRect.w),&(pRocket->shipRect.h));
    pRocket->shipRect.w /=4;
    pRocket->shipRect.h /=4;
    pRocket->xStart=pRocket->x=pRocket->shipRect.x=window_width*(number+1)/6-pRocket->shipRect.w/2;
    pRocket->yStart=pRocket->y=pRocket->shipRect.y=window_height/2-pRocket->shipRect.h/2;
    return pRocket;
}

int collideRocket(Rocket *pR1, Rocket *pR2){
    if(!pR1->alive || !pR2->alive) return 0;
    int collide = distance(pR1->shipRect.x+pR1->shipRect.w/2,pR1->shipRect.y+pR1->shipRect.h/2,pR2->shipRect.x+pR2->shipRect.w/2,pR2->shipRect.y+pR2->shipRect.h/2)<(pR1->shipRect.w+pR2->shipRect.w)/2;
    if(collide){
        pR1->alive = pR2->alive = 0;
    }
    return collide;
}

int hitRocket(Rocket *pShooter, Rocket *pTarget){
    if(!pShooter->alive || !pTarget->alive || !aliveBullet(pShooter->pBullet)) return 0;
    int collide = distance(xBullet(pShooter->pBullet),yBullet(pShooter->pBullet),pTarget->shipRect.x+pTarget->shipRect.w/2,pTarget->shipRect.y+pTarget->shipRect.h/2)<(pTarget->shipRect.w)/2;
    if(collide){
        pTarget->alive = 0;
        killBullet(pShooter->pBullet);
    }
    return collide;
}

float distance(int x1, int y1, int x2, int y2){
    return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

void turnLeft(Rocket *pRocket){
    pRocket->angle-=5;
}

void turnRight(Rocket *pRocket){
    pRocket->angle+=5;
}

void accelerate(Rocket *pRocket){
    float newvx, newvy;
    newvx = pRocket->vx+0.1*sin(pRocket->angle*2*M_PI/360);
    newvy = pRocket->vy-0.1*cos(pRocket->angle*2*M_PI/360);
    if(newvx*newvx+newvy*newvy<SQUAREOFMAXSPEED){
        pRocket->vx=newvx;
        pRocket->vy=newvy;
    }
}

void updateRocket(Rocket *pRocket){
    //if(!pRocket->alive) return;
    pRocket->x+=pRocket->vx;
    pRocket->y+=pRocket->vy;
    if(pRocket->x<0) pRocket->x+=pRocket->window_width;
    else if (pRocket->x>pRocket->window_width) pRocket->x-=pRocket->window_width;
    if(pRocket->y<0) pRocket->y+=pRocket->window_height;
    else if(pRocket->y>pRocket->window_height) pRocket->y-=pRocket->window_height;
    pRocket->shipRect.x=pRocket->x;
    pRocket->shipRect.y=pRocket->y;
    if(!aliveBullet(pRocket->pBullet)) return;
    updateBullet(pRocket->pBullet);
}

void drawRocket(Rocket *pRocket){
    if(aliveBullet(pRocket->pBullet)) drawBullet(pRocket->pBullet,pRocket->pRenderer);
    if(!pRocket->alive) return;
    SDL_RenderCopyEx(pRocket->pRenderer,pRocket->pTexture,NULL,&(pRocket->shipRect),pRocket->angle,NULL,SDL_FLIP_NONE);
}

void destroyRocket(Rocket *pRocket){
    SDL_DestroyTexture(pRocket->pTexture);
    destroyBullet(pRocket->pBullet);
    free(pRocket);
}

void fireRocket(Rocket *pRocket){
    if(!pRocket->alive || aliveBullet(pRocket->pBullet)) return;
    float bulletvx = 8*sin(pRocket->angle*2*M_PI/360);
    float bulletvy = -8*cos(pRocket->angle*2*M_PI/360);
    startBullet(pRocket->pBullet,pRocket->x+pRocket->shipRect.w/2,pRocket->y+pRocket->shipRect.h/2,pRocket->vx+bulletvx,pRocket->vy+bulletvy);
}

void resetRocket(Rocket *pRocket){
    pRocket->shipRect.x=pRocket->x=pRocket->xStart;
    pRocket->shipRect.y=pRocket->y=pRocket->yStart;
    pRocket->angle=0;
    pRocket->vx=pRocket->vy=0;
    pRocket->alive = 1;
    killBullet(pRocket->pBullet);
}

void getRocketSendData(Rocket *pRocket, RocketData *pRocketData){
    pRocketData->alive = pRocket->alive;
    pRocketData->angle = pRocket->angle;
    pRocketData->vx = pRocket->vx;
    pRocketData->vy = pRocket->vy;
    pRocketData->x = pRocket->x;
    pRocketData->y = pRocket->y;
    getBulletSendData(pRocket->pBullet,&(pRocketData->bData));
}

void updateRocketWithRecievedData(Rocket *pRocket, RocketData *pRocketData){
    pRocket->alive = pRocketData->alive;
    pRocket->angle = pRocketData->angle;
    pRocket->vx = pRocketData->vx;
    pRocket->vy = pRocketData->vy;
    pRocket->x = pRocketData->x;
    pRocket->y = pRocketData->y;
    updateBulletWithRecievedData(pRocket->pBullet,&(pRocketData->bData));
}