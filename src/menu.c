

#define WINDOW_WIDTH 1160
#define WINDOW_HEIGHT 700
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include "ship.h"
#include <SDL2/SDL_mixer.h>
#include "sound.h"
#include "text.h"
#include "stars.h"
#include "enemies.h"
#include "bullet.h"
#include "cannon.h"
#include "menu.h"

// Modify the showMenu function to include the IP address display
void showNetworkMenu(SDL_Renderer *renderer, TTF_Font *font, const char *ipAdress)
{
    // Menu box size and position
    int menuWidth = 800;
    int menuHeight = 250;
    int x = (WINDOW_WIDTH - menuWidth) / 2;
    int y = (WINDOW_HEIGHT - menuHeight) / 2 - 205;

    // Draw black box
    SDL_Rect box = {x, y, menuWidth, menuHeight};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
    SDL_RenderFillRect(renderer, &box);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 238,168,65, 255);
    SDL_RenderDrawRect(renderer, &box);

    // Draw text input box
    SDL_Rect textBox = {x+50, y+125, menuWidth-100, menuHeight-150};
    SDL_SetRenderDrawColor(renderer, 238,168,65, 0); // White
    SDL_RenderFillRect(renderer, &textBox);
    

    Text *prompt = createText(renderer, 238,168,65, font, "Enter IP Adress:", 600, 75);
    drawText(prompt);

    // Draw the IP address text if it exists
    if (strlen(ipAdress) > 0) {
        SDL_Color BLACK = {0, 0, 0};  // Text color
        SDL_Surface *ipSurface = TTF_RenderText_Solid(font, ipAdress, BLACK);
        if (ipSurface) {
            SDL_Texture *ipTexture = SDL_CreateTextureFromSurface(renderer, ipSurface);
            SDL_FreeSurface(ipSurface);
            
            if (ipTexture) {
                SDL_Rect ipRect = {x + 60, y + 145, ipSurface->w, ipSurface->h};
                SDL_RenderCopy(renderer, ipTexture, NULL, &ipRect);
                SDL_DestroyTexture(ipTexture);
            }
        }
    }
}

void showMapMenu(SDL_Renderer *renderer, TTF_Font *font)
{
    int menuWidth = 900;
    int menuHeight = 475;
    int x = (WINDOW_WIDTH - menuWidth) / 2;
    int y = (WINDOW_HEIGHT - menuHeight) / 2 + 50;
    TTF_Font *evenSmaller = TTF_OpenFont("arial.ttf", 25);

    SDL_Rect box = {x, y, menuWidth, menuHeight};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &box);

    //loop so it draws a bigger border.
    SDL_SetRenderDrawColor(renderer, 238, 168, 65, 255);
    for (int i = 0; i < 8; i++) {
        SDL_Rect border = {x - i, y - i, menuWidth + 2 * i, menuHeight + 2 * i};
        SDL_RenderDrawRect(renderer, &border);
    }

    // Title: Pick A Map
    Text *prompt = createText(renderer, 238, 167, 65, font, "Pick A Map", 600, 200);
    drawText(prompt);

    Text *EasyText = createText(renderer, 0, 255, 0, evenSmaller, "EASY", 375, 550);
    drawText(EasyText);

    
    Text *HardText = createText(renderer, 255, 0, 0, evenSmaller, "HARD", 750, 550);
    drawText(HardText);

}