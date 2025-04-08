#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WINDOW_WIDTH 1160
#define WINDOW_HEIGHT 700
#define SHIP_WIDTH 64
#define SHIP_HEIGHT 64
#define SHIP_SPEED 5

typedef struct {
    SDL_Window* pWindow;
    SDL_Renderer* pRenderer;
} Game;

int initiate(Game* pGame) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        return -1;
    }

    pGame->pWindow = SDL_CreateWindow("Space Ship", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!pGame->pWindow) {
        printf("Window Error: %s\n", SDL_GetError());
        return -1;
    }

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!pGame->pRenderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

void closeGame(Game* pGame) {
    SDL_DestroyRenderer(pGame->pRenderer);
    SDL_DestroyWindow(pGame->pWindow);
    IMG_Quit();
    SDL_Quit();
}

int loadAssets(SDL_Renderer* renderer, SDL_Texture** shipTexture) {
    SDL_Surface* surface = IMG_Load("resources/Ship.png");
    if (!surface) {
        printf("Failed to load image: %s\n", IMG_GetError());
        return -1;
    }

    *shipTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!*shipTexture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

void handleInput(SDL_Event* event, bool* isRunning, int* velX, int* velY) {
    if (event->type == SDL_QUIT) {
        *isRunning = false;
    }

    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        bool keyDown = (event->type == SDL_KEYDOWN);
        int value = keyDown ? SHIP_SPEED : 0;

        switch (event->key.keysym.sym) {
            case SDLK_w:
            case SDLK_UP:
                *velY = keyDown ? -value : 0;
                break;
            case SDLK_s:
            case SDLK_DOWN:
                *velY = keyDown ? value : 0;
                break;
            case SDLK_a:
            case SDLK_LEFT:
                *velX = keyDown ? -value : 0;
                break;
            case SDLK_d:
            case SDLK_RIGHT:
                *velX = keyDown ? value : 0;
                break;
        }
    }
}

void updatePosition(SDL_Rect* rect, int velX, int velY) {
    rect->x += velX;
    rect->y += velY;

    // Stay within window bounds
    if (rect->x < 0) rect->x = 0;
    if (rect->y < 0) rect->y = 0;
    if (rect->x + rect->w > WINDOW_WIDTH) rect->x = WINDOW_WIDTH - rect->w;
    if (rect->y + rect->h > WINDOW_HEIGHT) rect->y = WINDOW_HEIGHT - rect->h;
}

void render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* rect) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, texture, NULL, rect);
    SDL_RenderPresent(renderer);
}

void run(Game* pGame) {
    bool isRunning = true;
    SDL_Event event;

    SDL_Texture* shipTexture = NULL;
    if (loadAssets(pGame->pRenderer, &shipTexture) != 0) {
        return;
    }

    SDL_Rect shipRect = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, SHIP_WIDTH, SHIP_HEIGHT};
    int velX = 0, velY = 0;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            handleInput(&event, &isRunning, &velX, &velY);
        }

        updatePosition(&shipRect, velX, velY);
        render(pGame->pRenderer, shipTexture, &shipRect);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyTexture(shipTexture);
}

int main(int argc, char** argv) {
	printf("Starting program...\n");
    Game game = {NULL, NULL};

    if (initiate(&game) != 0) {
		printf("Init failed\n");
        return -1;
    }

    run(&game);
    closeGame(&game);
    return 0;
}
