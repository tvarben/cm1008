#include "../include/stars.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>

struct stars {
    int nr_of_stars;
    SDL_Point *pStarList;
};

static void drawStar1(SDL_Point *pPoint, SDL_Renderer *pRenderer);
static void drawStar2(SDL_Point *pPoint, SDL_Renderer *pRenderer);
static void drawStar3(SDL_Point *pPoint, SDL_Renderer *pRenderer);

Stars *createStars(int nr_of_stars, int width, int height) {
    srand(1);
    Stars *s = malloc(sizeof(struct stars));
    s->pStarList = malloc(nr_of_stars * sizeof(SDL_Point));
    s->nr_of_stars = nr_of_stars;
    for (int i = 0; i < nr_of_stars; i++) {
        s->pStarList[i].x = rand() % width;
        s->pStarList[i].y = rand() % height;
    }
    return s;
}

void drawStars(Stars *pstars, SDL_Renderer *pRenderer) {
    for (int i = 0; i < pstars->nr_of_stars; i++) {
        if (i % 3 == 0)
            drawStar1(&(pstars->pStarList[i]), pRenderer);
        else if (i % 3 == 1)
            drawStar2(&(pstars->pStarList[i]), pRenderer);
        else
            drawStar3(&(pstars->pStarList[i]), pRenderer);
    }
}

// white dots (circles)
static void drawStar1(SDL_Point *pPoint, SDL_Renderer *pRenderer) {
    SDL_RenderDrawPoint(pRenderer, pPoint->x, pPoint->y - 1);
    SDL_RenderDrawPoint(pRenderer, pPoint->x + 1, pPoint->y - 1);
    SDL_RenderDrawPoint(pRenderer, pPoint->x - 1, pPoint->y);
    SDL_RenderDrawPoint(pRenderer, pPoint->x, pPoint->y);
    SDL_RenderDrawPoint(pRenderer, pPoint->x + 1, pPoint->y);
    SDL_RenderDrawPoint(pRenderer, pPoint->x + 2, pPoint->y);
    SDL_RenderDrawPoint(pRenderer, pPoint->x - 1, pPoint->y + 1);
    SDL_RenderDrawPoint(pRenderer, pPoint->x, pPoint->y + 1);
    SDL_RenderDrawPoint(pRenderer, pPoint->x + 1, pPoint->y + 1);
    SDL_RenderDrawPoint(pRenderer, pPoint->x + 2, pPoint->y + 1);
    SDL_RenderDrawPoint(pRenderer, pPoint->x, pPoint->y + 2);
    SDL_RenderDrawPoint(pRenderer, pPoint->x + 1, pPoint->y + 2);
}

// Lines radiating outward from center (5-pointed)
static void drawStar2(SDL_Point *pPoint, SDL_Renderer *pRenderer) {
    int x1 = pPoint->x;
    int y1 = pPoint->y;

    SDL_RenderDrawLine(pRenderer, x1, y1, x1, y1 - 3);
    SDL_RenderDrawLine(pRenderer, x1, y1, x1 + 3, y1);
    SDL_RenderDrawLine(pRenderer, x1, y1, x1, y1 + 3);
    SDL_RenderDrawLine(pRenderer, x1, y1, x1 - 3, y1);
    SDL_RenderDrawLine(pRenderer, x1, y1, x1 + 2, y1 - 2);
    SDL_RenderDrawLine(pRenderer, x1, y1, x1 - 2, y1 - 2);
    SDL_RenderDrawLine(pRenderer, x1, y1, x1 + 2, y1 + 2);
    SDL_RenderDrawLine(pRenderer, x1, y1, x1 - 2, y1 + 2);
}

static void drawStar3(SDL_Point *pPoint, SDL_Renderer *pRenderer) {
    int x = pPoint->x;
    int y = pPoint->y;

    SDL_RenderDrawPoint(pRenderer, x, y);
    SDL_RenderDrawPoint(pRenderer, x + 1, y);
    SDL_RenderDrawPoint(pRenderer, x, y + 1);
    SDL_RenderDrawPoint(pRenderer, x + 1, y + 1);
}

void destroyStars(Stars *pStars) {
    free(pStars->pStarList);
    free(pStars);
}