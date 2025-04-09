#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include "ship.h"

#define WINDOW_WIDTH 1160
#define WINDOW_HEIGHT 700

enum GameState { START, ONGOING, GAME_OVER };
typedef enum GameState GameState;

typedef struct {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ship *pShip;
    GameState state;
} Game;

int initiate(Game *pGame) 
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return 0;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        printf("SDL_image Init Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 0;
    }

    pGame->pWindow = SDL_CreateWindow("",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!pGame->pWindow) {
        printf("Window Error: %s\n", SDL_GetError());
        return 0;
    }

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        return 0;
    }

    pGame->pShip = createShip(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!pGame->pShip) {
        printf("Ship creation failed.\n");
        return 0;
    }

    pGame->state = START;
    return 1;
}

void run(Game *pGame) {
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            } else if (pGame->state == START && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                resetShip(pGame->pShip);
                pGame->state = ONGOING;                 // set game state to ONGOING and exit the loop
            } else if (pGame->state == ONGOING && (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)) {
                handleShipEvent(pGame->pShip, &event);  // track which keys are pressed
            }
        }

        if (pGame->state == ONGOING) 
        {
            updateShipVelocity(pGame->pShip);           // resolve velocity based on key states
            updateShip(pGame->pShip);
            SDL_SetRenderDrawColor(pGame->pRenderer, 30, 30, 30, 255);
            SDL_RenderClear(pGame->pRenderer);
            drawShip(pGame->pShip);
            SDL_RenderPresent(pGame->pRenderer);
        } 
        else if (pGame->state == START) 
        {
            SDL_SetRenderDrawColor(pGame->pRenderer, 10, 10, 40, 255);
            SDL_RenderClear(pGame->pRenderer);
            drawShip(pGame->pShip);
            SDL_RenderPresent(pGame->pRenderer);
        }
    }
}

void closeGame(Game *pGame) {
    if (pGame->pShip) destroyShip(pGame->pShip);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char** argv) {
    Game game = {NULL, NULL, NULL, START};
    if (!initiate(&game)) return 1;

    run(&game);
    closeGame(&game);
    return 0;
}
