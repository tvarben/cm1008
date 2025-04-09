#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 1160
#define WINDOW_HEIGHT 700

int volume = 50;
void setVolume(int v) { volume = (MIX_MAX_VOLUME * v) / 100; }

typedef struct {
  SDL_Window *pWindow;
  SDL_Renderer *pRenderer;
  Mix_Music *pBackgroundMusic;

} Game;

int initiate(Game *pGame) {

  Mix_Init(MIX_INIT_WAVPACK);
  SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_AUDIO);
  Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096);

  pGame->pWindow =
      SDL_CreateWindow("Hello World", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, 0);
  pGame->pBackgroundMusic =
      /*Mix_LoadMUS("/home/tvarben/School/cm1008/resources/sound/techno.wav");*/
      Mix_LoadMUS("resources/sound/techno.wav");
  if (!pGame->pBackgroundMusic) {
    printf("Failed to laod music. SDL_mixer Error %s\n", Mix_GetError());
    return -1;
  }
  Mix_VolumeMusic(volume);
  Mix_PlayMusic(pGame->pBackgroundMusic, -1); // plays background music once
                                              //
}

void closeGame(Game *pGame) {
  SDL_DestroyWindow(pGame->pWindow);
  SDL_DestroyRenderer(pGame->pRenderer);
  SDL_Quit();
}

void run(Game *pGame) {
  bool isRunning = true;
  SDL_Event event;

  while (isRunning) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        isRunning = false;
        break;
      }
    }
  }
}

int main(int argv, char **args) {
  Game game = {NULL, NULL, NULL};
  initiate(&game);

  bool isRunning = true;
  run(&game);

  closeGame(&game);
  return 0;
}
