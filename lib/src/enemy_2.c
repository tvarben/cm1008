#include "../../lib/include/enemy_2.h"
#include "../../lib/include/ship_data.h"
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct enemyImage_2 {
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
};

struct enemy2 {
    float x, y, vx, vy;
    int health, damage, window_width, window_height;
    bool active;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect rect;
    SDL_Rect rectHitbox;
};

static void getStartValues_2(Enemy_2 *pEnemy2);
static void getStartValuesFromServer_2(Enemy_2 *pEnemy, Enemy_2_Data enemyData);

EnemyImage_2 *initiateEnemy_2(SDL_Renderer *pRenderer) {
    static EnemyImage_2 *pEnemyImage2 = NULL;
    if (pEnemyImage2 == NULL) {
        pEnemyImage2 = malloc(sizeof(struct enemyImage_2));
        SDL_Surface *surface = IMG_Load("../lib/resources/enemy2.png");

        if (!surface) {
            printf("Error: %s\n", SDL_GetError());
            return NULL;
        }

        pEnemyImage2->pRenderer = pRenderer;
        pEnemyImage2->pTexture = SDL_CreateTextureFromSurface(pRenderer, surface);
        SDL_FreeSurface(surface);

        if (!pEnemyImage2->pTexture) {
            printf("Error: %s\n", SDL_GetError());
            return NULL;
        }
    }
    return pEnemyImage2;
}

Enemy_2 *createEnemy_2(EnemyImage_2 *pEnemyImage2, int window_width, int window_height) {
    Enemy_2 *pEnemy2 = malloc(sizeof(struct enemy2));
    pEnemy2->pRenderer = pEnemyImage2->pRenderer;
    pEnemy2->pTexture = pEnemyImage2->pTexture;
    pEnemy2->window_width = window_width;
    pEnemy2->window_height = window_height;
    SDL_QueryTexture(pEnemyImage2->pTexture, NULL, NULL, &(pEnemy2->rect.w), &(pEnemy2->rect.h));
    getStartValues_2(pEnemy2);
    pEnemy2->active = true;
    return pEnemy2;
}

Enemy_2 *createEnemy_2_OnClients(EnemyImage_2 *pEnemyImage, int window_width, int window_height, Enemy_2_Data enemyData) {
    Enemy_2 *pEnemy = malloc(sizeof(struct enemy2));
    pEnemy->pRenderer = pEnemyImage->pRenderer;
    pEnemy->pTexture = pEnemyImage->pTexture;
    pEnemy->window_width = window_width;
    pEnemy->window_height = window_height;
    SDL_QueryTexture(pEnemyImage->pTexture, NULL, NULL, &(pEnemy->rect.w), &(pEnemy->rect.h));
    getStartValuesFromServer_2(pEnemy, enemyData);
    return pEnemy;
}

static void getStartValues_2(Enemy_2 *pEnemy2) {
    int startSpawnOnTheLeft = rand() % 2; // 0 or 1
    pEnemy2->rectHitbox.x = pEnemy2->rect.x + 10;
    pEnemy2->rectHitbox.y = pEnemy2->rect.y + 10;
    pEnemy2->rectHitbox.w = pEnemy2->rect.w - 50;
    pEnemy2->rectHitbox.h = pEnemy2->rect.h - 50;
    pEnemy2->damage = 1;
    pEnemy2->health = 50; // changed health
    // float speed = rand() % (50 - 15 + 1) + 15;
    float speed = rand() % (15 - 10 + 1) + 10;
    if (startSpawnOnTheLeft == 1) {
        pEnemy2->x = pEnemy2->window_width;
        pEnemy2->y = rand() % (pEnemy2->window_height - pEnemy2->rect.h);

        pEnemy2->vx = -speed; // rakt åt vänster
        pEnemy2->vy = 0;      // ingen rörelse i y-led
    } else {
        pEnemy2->x = 0; // also spawn at left
        pEnemy2->y = rand() % (pEnemy2->window_height - pEnemy2->rect.h);

        pEnemy2->vx = speed; // move right
        pEnemy2->vy = 0;
    }
}

static void getStartValuesFromServer_2(Enemy_2 *pEnemy, Enemy_2_Data enemyData) {
    pEnemy->rectHitbox.x = pEnemy->rect.x + 10;
    pEnemy->rectHitbox.y = pEnemy->rect.y + 10;
    pEnemy->rectHitbox.w = pEnemy->rect.w - 50;
    pEnemy->rectHitbox.h = pEnemy->rect.h - 50;
    pEnemy->damage = 1;
    pEnemy->health = 50;
    pEnemy->x = enemyData.x;
    pEnemy->y = enemyData.y;
    pEnemy->active = enemyData.active;
}

SDL_Rect getRectEnemy_2(Enemy_2 *pEnemy2) {
    if (pEnemy2->active == true) {
        return pEnemy2->rectHitbox;
    }
    SDL_Rect empty = {0, 0, 0, 0};
    return empty;
}

void updateEnemy_2(Enemy_2 *pEnemy2) {
    int amplitude = 5;
    float frequency = 0.03;
    /*double doubleX = pEnemy2->x;*/
    float sinx = sin(pEnemy2->x * frequency) * amplitude;
    if (pEnemy2->active == true) {
        pEnemy2->x += pEnemy2->vx * 0.1; // test values
        pEnemy2->y += sin(pEnemy2->x * frequency) * amplitude;
        if (pEnemy2->x > pEnemy2->window_width || pEnemy2->x + pEnemy2->rect.w < 0 ||
            pEnemy2->y > pEnemy2->window_height || pEnemy2->y + pEnemy2->rect.h < 0) {
            getStartValues_2(pEnemy2);
            pEnemy2->active = false;
            return;
        }
        pEnemy2->rect.x = pEnemy2->x;
        pEnemy2->rect.y = pEnemy2->y;
        pEnemy2->rectHitbox.x = pEnemy2->rect.x + 40;
        pEnemy2->rectHitbox.y = pEnemy2->rect.y + 20;
    }
}

void updateEnemy_2_OnClients(Enemy_2 *pEnemy, Enemy_2_Data enemyData) {
    pEnemy->x = enemyData.x;
    pEnemy->y = enemyData.y;
    pEnemy->active = enemyData.active;
    pEnemy->rect.x = pEnemy->x;
    pEnemy->rect.y = pEnemy->y;
    pEnemy->rectHitbox.x = pEnemy->rect.x + 40;
    pEnemy->rectHitbox.y = pEnemy->rect.y + 20;
}

void drawEnemy_2(Enemy_2 *pEnemy2) {
    if (pEnemy2->active == true) {
        SDL_RenderCopyEx(pEnemy2->pRenderer, pEnemy2->pTexture, NULL, &(pEnemy2->rect), 0, NULL, SDL_FLIP_NONE); // made 0 to not rotate enemy.png
    }
}

void destroyEnemy_2(Enemy_2 *pEnemy2) {
    free(pEnemy2);
}

void destroyEnemyImage_2(EnemyImage_2 *pEnemyImage2) {
    SDL_DestroyTexture(pEnemyImage2->pTexture);
}

void disableEnemy_2(Enemy_2 *pEnemy2) {
    if (pEnemy2->health <= 0) {
        pEnemy2->active = false;
    }
}

void damageEnemy_2(Enemy_2 *pEnemy2, int damage, int i) {
    pEnemy2->health -= damage;
    if (pEnemy2->health <= 0 && pEnemy2->active == true) {
        pEnemy2->active = false;
        printf("Enemy nr %d dead\n", i);
    }
    if (pEnemy2->health < 0) pEnemy2->health = 0;
}

bool isInWindow_2(Enemy_2 *pEnemy2) {
    if (pEnemy2->x > pEnemy2->window_width || pEnemy2->x + pEnemy2->rect.w < 0 ||
        pEnemy2->y > pEnemy2->window_height || pEnemy2->y + pEnemy2->rect.h < 0) {
        disableEnemy_2(pEnemy2);
        return false;
    } else {
        return true;
    }
}

bool isEnemy_2Active(Enemy_2 *pEnemy2) {
    if (pEnemy2->active == false) {
        return false;
    } else {
        return true;
    }
}

void printEnemy_2Health(Enemy_2 *pEnemy2) {
    if (pEnemy2->active == true) {
        printf("Health: %d\n", pEnemy2->health);
    }
}

void getEnemy_2_DataPackage(Enemy_2 *pEnemy, Enemy_2_Data *pEnemyData) {
    pEnemyData->x = pEnemy->x;
    pEnemyData->y = pEnemy->y;
    pEnemyData->active = pEnemy->active;
}
