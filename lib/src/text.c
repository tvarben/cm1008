#include "../include/text.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

struct text {
    SDL_Rect rect;
    SDL_Texture *pTexture;
    SDL_Renderer *pRenderer;
};

Text *createText(SDL_Renderer *pRenderer, int r, int g, int b, TTF_Font *pFont, char *pString, int x, int y) {
    Text *pText = malloc(sizeof(struct text));
    pText->pRenderer = pRenderer;
    SDL_Color color = {r, g, b};
    SDL_Surface *pSurface = TTF_RenderText_Solid(pFont, pString, color);
    if (!pSurface) {
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    pText->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if (!pText->pTexture) {
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_QueryTexture(pText->pTexture, NULL, NULL, &pText->rect.w, &pText->rect.h);
    pText->rect.x = x - pText->rect.w / 2;
    pText->rect.y = y - pText->rect.h / 2;

    return pText;
}

void setTextColor(Text *pText, int r, int g, int b, TTF_Font *pFont, const char *pString) {
    SDL_Color color = {r, g, b};
    SDL_Surface *pSurface = TTF_RenderText_Solid(pFont, pString, color);
    if (!pSurface) {
        printf("Error: %s\n", SDL_GetError());
        return;
    }

    SDL_DestroyTexture(pText->pTexture);
    pText->pTexture = SDL_CreateTextureFromSurface(pText->pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
}

const SDL_Rect *getTextRect(Text *pText) {
    return &pText->rect;
}

void drawText(Text *pText) {
    SDL_RenderCopy(pText->pRenderer, pText->pTexture, NULL, &pText->rect);
}

void destroyText(Text *pText) {
    SDL_DestroyTexture(pText->pTexture);
    free(pText);
}