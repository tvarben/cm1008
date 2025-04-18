#include "../include/bullet.h"
#include "../include/cannon.h"
#include "../include/ship.h"
#include "../include/sound.h"
#include "../include/text.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>

#define WINDOW_WIDTH 1160
#define WINDOW_HEIGHT 700
#define MUSIC_FILEPATH "./resources/music.wav"

enum GameState { START, ONGOING, GAME_OVER };
typedef enum GameState GameState;

typedef struct {
  SDL_Window *pWindow;
  SDL_Renderer *pRenderer;
  Ship *pShip;
  Cannon *pCannon;
  GameState state;
  Mix_Music *pMusic;
  TTF_Font *pFont;
  Text *pStartText, *pGameName, *pExitText;
} Game;

int initiate(Game *pGame) {

  Mix_Init(MIX_INIT_WAVPACK);
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
    printf("SDL Init Error: %s\n", SDL_GetError());
    return 0;
  }

  if (IMG_Init(IMG_INIT_PNG) == 0) {
    printf("SDL_image Init Error: %s\n", IMG_GetError());
    SDL_Quit();
    return 0;
  }

  if (TTF_Init() != 0) {
    printf("Error: %s\n", TTF_GetError());
    SDL_Quit();
    return 0;
  }

  pGame->pWindow =
      SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (!pGame->pWindow) {
    printf("Window Error: %s\n", SDL_GetError());
    return 0;
  }

  pGame->pRenderer = SDL_CreateRenderer(
      pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!pGame->pRenderer) {
    printf("Renderer Error: %s\n", SDL_GetError());
    return 0;
  }

  pGame->pFont = TTF_OpenFont("resources/Outwrite.ttf", 100);
  if (!pGame->pFont) {
    printf("Error: %s\n", TTF_GetError());
    return 0;
  }

  pGame->pStartText =
      createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Start [1]",
                 WINDOW_WIDTH / 3, WINDOW_HEIGHT / 2 + 100);
  pGame->pGameName =
      createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "SpaceShooter",
                 WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4);
  pGame->pExitText =
      createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Exit [2]",
                 WINDOW_WIDTH / 1.5, WINDOW_HEIGHT / 2 + 100);

  pGame->pShip = createShip(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2,
                            pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
  pGame->pCannon = createCannon(pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);

  if (!pGame->pCannon) {
    printf("Cannon creation failed.\n");
    return 0;
  }

  if (!pGame->pShip) {
    printf("Ship creation failed.\n");
    return 0;
  }

  pGame->pMusic = initMusic(MUSIC_FILEPATH);
  if (!pGame->pMusic) {
    printf("Error: %s\n", Mix_GetError());
    return 0;
  }

  pGame->state = START;
  return 1;
}

void run(Game *pGame) {
  bool isRunning = true;
  SDL_Event event;
  playMusic(pGame->pMusic, -1);

  Uint32 last_time = SDL_GetTicks(); // timer for rendering bullets
  render_projectiles(pGame->pRenderer);

  while (isRunning) {
    Uint32 current_time = SDL_GetTicks(); // needed for shooting
    float delta_time = (current_time - last_time) /
                       1000.0f; // calculates time passed since last frame
    last_time = current_time;   // needed for shooting
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        isRunning = false;
      } else if (pGame->state == START && event.type == SDL_KEYDOWN &&
                 event.key.keysym.scancode == SDL_SCANCODE_1) {
        resetShip(pGame->pShip);
        resetCannon(pGame->pCannon);
        pGame->state = ONGOING;
      } else if (pGame->state == START && event.type == SDL_KEYDOWN &&
                 event.key.keysym.scancode == SDL_SCANCODE_2) {
        isRunning = false;
      } else if (pGame->state == ONGOING &&
                 (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)) {
        handleCannonEvent(pGame->pCannon, &event); // what makes cannon shoot
        handleShipEvent(pGame->pShip, &event);
      }
    }

    if (pGame->state == ONGOING) {
      update_projectiles(
          delta_time); // update based on time since last frame passed
      if (Mix_PlayingMusic())
        Mix_HaltMusic();
      updateShipVelocity(pGame->pShip);
      updateShip(pGame->pShip);
      updateCannon(pGame->pCannon, pGame->pShip);

      SDL_SetRenderDrawColor(pGame->pRenderer, 30, 30, 30, 255);
      SDL_RenderClear(pGame->pRenderer);
      drawShip(pGame->pShip);
      drawCannon(pGame->pCannon);
      render_projectiles(pGame->pRenderer); // test
      SDL_RenderPresent(pGame->pRenderer);
    } else if (pGame->state == START) {
      SDL_SetRenderDrawColor(pGame->pRenderer, 30, 30, 30, 255);
      SDL_RenderClear(pGame->pRenderer);
      drawText(pGame->pStartText);
      drawText(pGame->pExitText);
      drawText(pGame->pGameName);
      SDL_RenderPresent(pGame->pRenderer);
    }
  }
}

void closeGame(Game *pGame) {
  if (pGame->pShip)
    destroyShip(pGame->pShip);
  destroyCannon(pGame->pCannon);
  if (pGame->pRenderer)
    SDL_DestroyRenderer(pGame->pRenderer);
  if (pGame->pWindow)
    SDL_DestroyWindow(pGame->pWindow);

  if (pGame->pStartText)
    destroyText(pGame->pStartText);
  if (pGame->pFont)
    TTF_CloseFont(pGame->pFont);

  closeMusic(pGame->pMusic);
  IMG_Quit();
  SDL_Quit();
}

int main(int argc, char **argv) {
  Game game = {NULL, NULL, NULL, NULL, START};
  if (!initiate(&game))
    return 1;

  run(&game);
  closeGame(&game);
  return 0;
}
