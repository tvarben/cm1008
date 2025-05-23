#ifndef MENU_H
#define MENU_H
#include <SDL2/SDL_ttf.h>

#include <SDL2/SDL.h>

char *enterIPAddress(SDL_Renderer *renderer, TTF_Font *font);
void drawModifierMenu(SDL_Renderer *pRenderer, TTF_Font *pFont);
void DrawModifiersToMakeGameEasier(SDL_Renderer *pRenderer, TTF_Font *pFont);
void DrawModifiersToMakeGameHarder(SDL_Renderer *pRenderer, TTF_Font *pFont);
void drawSoreModifier(SDL_Renderer *pRenderer);
#endif
