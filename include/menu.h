#ifndef Menu_H
#include <SDL2/SDL_ttf.h>
#define MENU_H
void showNetworkMenu(SDL_Renderer *renderer, TTF_Font *font,
                     const char *ipAdress);
void showMapMenu(SDL_Renderer *renderer, TTF_Font *font);
#endif
