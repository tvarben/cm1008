

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
    
    // Draw white border
    SDL_SetRenderDrawColor(renderer, 238,168,65, 255);
    SDL_RenderDrawRect(renderer, &box);

    // Draw text input box
    SDL_Rect textBox = {x+50, y+125, menuWidth-100, menuHeight-150};
    SDL_SetRenderDrawColor(renderer, 238,168,65, 0); // White
    SDL_RenderFillRect(renderer, &textBox);

    // Draw title text
    SDL_Color textColor = {238,168,65};
    SDL_Surface *surface = TTF_RenderText_Solid(font, "Enter IP Adress: ", textColor);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect = {
        x + (menuWidth - surface->w) / 2,
        y + 30,
        surface->w,
        surface->h
    };
    
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_DestroyTexture(texture);

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

    SDL_SetRenderDrawColor(renderer, 238, 168, 65, 255);
    for (int i = 0; i < 8; i++) {
        SDL_Rect border = {x - i, y - i, menuWidth + 2 * i, menuHeight + 2 * i};
        SDL_RenderDrawRect(renderer, &border);
    }
    SDL_Color textColor = {238, 168, 65};
    SDL_Color RED = {255,0,0};
    SDL_Color GREEN = {0, 255, 0};


    // Title: Pick A Map
    SDL_Surface *surface = TTF_RenderText_Solid(font, "Pick A Map", textColor);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect textRect = {
        x + (menuWidth - surface->w) / 2,
        y + 20,
        surface->w,
        surface->h
    };
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // EASY label (left)
    surface = TTF_RenderText_Solid(evenSmaller, "EASY", GREEN);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect easyRect = {
        x + 210,                    // far left
        y + menuHeight - 100,       // near bottom
        surface->w,
        surface->h
    };
    SDL_RenderCopy(renderer, texture, NULL, &easyRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // HARD label (right)
    surface = TTF_RenderText_Solid(evenSmaller, "HARD", RED);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect hardRect = {
        x + menuWidth - surface->w - 225,  // far right
        y + menuHeight - 100,
        surface->w,
        surface->h
    };
    SDL_RenderCopy(renderer, texture, NULL, &hardRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}