#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

typedef struct {
	SDL_Window *pWindow;
	SDL_Renderer *pRenderer;
}Game;

int initiate(Game *pGame) {
	
	SDL_Init(SDL_INIT_EVERYTHING);

	pGame->pWindow = SDL_CreateWindow("Hello World", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, 0);
}

void close(Game *pGame) {
	SDL_DestroyWindow(pGame->pWindow);
	SDL_DestroyRenderer(pGame->pRenderer);
	SDL_Quit();
}

void run(Game *pGame) {
	bool isRunning = true;
	SDL_Event event;

	while(isRunning) {
		while(SDL_PollEvent(&event)){
			switch(event.type) {
				case SDL_QUIT:
					isRunning = false;
					break;
			}
		}
	}
}

int main(int argv, char** args) {
	Game game = {NULL, NULL};
	initiate(&game);
	
	run(&game);

	close(&game);
}
