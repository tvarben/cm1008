#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

struct game {
  SDL_Window *pWindow;
  SDL_Renderer *pRenderer;
};
typedef struct game Game;

int initiate(Game *pGame);
void close_all(Game *pGame);

int main() {
  Game game = {0};
  if (!initiate(&game)) return 1;

  SDL_Delay(5000);

  close_all(&game);
}

int initiate(Game *pGame) {
  // Initiate SDL2
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
    printf("Failed to initiate SDL: %s\n",SDL_GetError());
    SDL_Quit();
    return 0;
  }

  // Initiate Window
  pGame->pWindow = SDL_CreateWindow("Hello World", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256, 240, 0);
  if (!pGame->pWindow) {
    printf("Error creating window: %s\n",SDL_GetError());
    SDL_Quit();
    return 0;
  }

  // Initiate Renderer
  Uint32 render_flags = SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC;

  pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow,-1,render_flags);
  if (!pGame->pRenderer) {
    printf("Error creating renderer: %s\n",SDL_GetError());
    SDL_DestroyWindow(pGame->pWindow);
    SDL_Quit();
    return 0;
  }
  
  return 1;
}

void close_all(Game *pGame) {
  if (pGame == NULL) {
    SDL_Quit();
    return;
  } 

  SDL_DestroyRenderer(pGame->pRenderer);
  SDL_DestroyWindow(pGame->pWindow);
  SDL_Quit();  
}