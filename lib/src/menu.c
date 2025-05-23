#include "../include/ship.h"
#include "../include/ship_data.h"
#include "../include/text.h"
#include "ship.h"
#include "ship_data.h"
#include "text.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

char *enterIPAddress(SDL_Renderer *renderer, TTF_Font *font) {
    static char ip[32] = "";
    SDL_StartTextInput();
    SDL_Event event;
    bool done = false;

    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return NULL;
            if (event.type == SDL_TEXTINPUT) {
                if (strlen(ip) < 31) strcat(ip, event.text.text);
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(ip) > 0)
                    ip[strlen(ip) - 1] = '\0';
                if (event.key.keysym.sym == SDLK_RETURN) done = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_Rect box = {200, 300, 760, 80};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &box);
        SDL_SetRenderDrawColor(renderer, 238, 168, 65, 255);
        SDL_RenderDrawRect(renderer, &box);

        SDL_Color color = {255, 255, 255};
        SDL_Surface *surf = TTF_RenderText_Solid(font, ip, color);
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect rect = {box.x + 10, box.y + 25, surf->w, surf->h};
        SDL_RenderCopy(renderer, tex, NULL, &rect);
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    return ip;
}


void drawModifierMenu(SDL_Renderer *pRenderer, TTF_Font *pFont){
    int menuWidth = 900;
    int menuHeight = 500;
    int x = (WINDOW_WIDTH - menuWidth) / 2;
    int y = (WINDOW_HEIGHT - menuHeight) / 2  -50;

    SDL_Rect box = {x, y, menuWidth, menuHeight};
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
    SDL_RenderFillRect(pRenderer, &box); 
    SDL_SetRenderDrawColor(pRenderer, 238, 168, 65, 255);
    SDL_RenderDrawRect(pRenderer, &box);

    int innerWidth = menuWidth / 3 + 50;
    int innerHeight = menuHeight / 5;
    int innerX = x + (menuWidth - innerWidth) / 2;
    int innerY = y + (menuHeight - innerHeight) / 2 + 135;

    SDL_Rect innerBox = {innerX, innerY, innerWidth, innerHeight};
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);  //
    SDL_RenderFillRect(pRenderer, &innerBox);
    SDL_SetRenderDrawColor(pRenderer, 238, 168, 65, 255); 
    SDL_RenderDrawRect(pRenderer, &innerBox);
}

void DrawModifiersToMakeGameEasier(SDL_Renderer *pRenderer, TTF_Font *pFont){
    int menuWidth = 250;
    int menuHeight = 650;
    int x = 25;
    int y = (WINDOW_HEIGHT - menuHeight) / 2 ;

    SDL_Rect box = {x, y, menuWidth, menuHeight};
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
    SDL_RenderFillRect(pRenderer, &box); 
    SDL_SetRenderDrawColor(pRenderer, 238, 168, 65, 255);
    SDL_RenderDrawRect(pRenderer, &box);
}

void DrawModifiersToMakeGameHarder(SDL_Renderer *pRenderer, TTF_Font *pFont){
    int menuWidth = 250;
    int menuHeight = 650;
    int x = (WINDOW_WIDTH - menuWidth) - 20;
    int y = (WINDOW_HEIGHT - menuHeight) / 2;

    SDL_Rect box = {x, y, menuWidth, menuHeight};
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
    SDL_RenderFillRect(pRenderer, &box); 
    SDL_SetRenderDrawColor(pRenderer, 238, 168, 65, 255);
    SDL_RenderDrawRect(pRenderer, &box);
}

void drawSoreModifier(SDL_Renderer *pRenderer) { //idk a good name for this one its just the black box and border
    int hatWidth = 245;
    int hatHeight = 100;
    int x = 15;
    int y = (WINDOW_HEIGHT) - 120;

    SDL_Rect hatBox = {x, y, hatWidth, hatHeight};
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);  // Fill: black
    SDL_RenderFillRect(pRenderer, &hatBox);
    SDL_SetRenderDrawColor(pRenderer, 238, 168, 65, 255);  // Border: orange
    SDL_RenderDrawRect(pRenderer, &hatBox);
}
