#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_net.h>
#include "ship.h"
#include "sound.h"
#include "text.h"

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 400
#define MUSIC_FILEPATH "../lib/resources/music.wav"
#define MAX_PLAYERS 2


enum GameState { START, ONGOING, GAME_OVER };
typedef enum GameState GameState;

typedef struct {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ship *pShip;
    GameState state;
    Mix_Music *pMusic;
	TTF_Font *pFont;
	Text *pStartText, *pGameName, *pExitText;

    UDPsocket pSocket;
    IPaddress serverAddress;
    UDPpacket *pPacket;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void closeGame(Game *pGame);

int main(int argc, char** argv) {
    Game game = {0};
    if (!initiate(&game)) return 1;

    run(&game);
    closeGame(&game);
    return 0;
}


int initiate(Game *pGame) {
    srand(time(NULL));
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
	if(TTF_Init()!=0) {
        printf("Error: %s\n",TTF_GetError());
        SDL_Quit();
        return 0;
    }
    if (SDLNet_Init()) {
        printf("SDLNet_Init: %s\n", SDLNet_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    pGame->pWindow = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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

	pGame->pFont = TTF_OpenFont("../lib/resources/arial.ttf", 100);
    if(!pGame->pFont ) {
        printf("Error: %s\n",TTF_GetError());
        return 0;
    }

	pGame->pStartText = createText(pGame->pRenderer,238,168,65,pGame->pFont,
                                "Start [1]",WINDOW_WIDTH/3,WINDOW_HEIGHT/2+100);
    pGame->pGameName = createText(pGame->pRenderer,238,168,65,pGame->pFont,
                                "SpaceShooter",WINDOW_WIDTH/2,WINDOW_HEIGHT/4);
    pGame->pExitText = createText(pGame->pRenderer,238,168,65,pGame->pFont,
                                "Exit [2]",WINDOW_WIDTH/1.5,WINDOW_HEIGHT/2+100);
    if(!pGame->pFont){
        printf("Error: %s\n",TTF_GetError());
        return 0;
    } 
    if (!(pGame->pSocket = SDLNet_UDP_Open(2000))) {
        printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }
    if (!(pGame->pPacket = SDLNet_AllocPacket(512))) {
        printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 0;
    }

    pGame->pPacket->address.host = pGame->serverAddress.host;
    pGame->pPacket->address.port = pGame->serverAddress.port;

    pGame->pShip = createShip(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!pGame->pShip) {
        printf("Ship creation failed.\n");
        return 0;
    }

    if (!initMusic(&pGame->pMusic, MUSIC_FILEPATH)) {
        printf("Error: %s\n",Mix_GetError());
        return 0;
    }

    pGame->state = START;
    return 1;
}

void run(Game *pGame) {
    bool isRunning = true;
    printf("Server is listening on port 2000...\n");
    SDL_Event event;

    while (isRunning) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }
        SDL_SetRenderDrawColor(pGame->pRenderer, 30, 30, 30, 255);
        SDL_RenderClear(pGame->pRenderer);   
        
        drawText(pGame->pStartText);
        SDL_RenderPresent(pGame->pRenderer);
        
        if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
            printf("Server recieved a command from %x:%d: %s\n",
                                        pGame->pPacket->address.host, 
                                        pGame->pPacket->address.port, 
                                        (char*)pGame->pPacket->data);
            
            strcpy((char*)pGame->pPacket->data, "Har far du svar fran server.");
            pGame->pPacket->len = strlen((char*)pGame->pPacket->data) + 1;
            SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
            
            //strcpy((char*) pGame->pPacket->data, "Message recieved!!!!!!!!!!!!!!!!");
            //pGame->pPacket->len = strlen((char*)pGame->pPacket->data) +1;
        
            //SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
            //printf("Response sent.\n");
        }
        SDL_Delay(8);
    }
}

void closeGame(Game *pGame) {
    if (pGame->pShip) destroyShip(pGame->pShip);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);

    if (pGame->pStartText) destroyText(pGame->pStartText);
    if (pGame->pFont) TTF_CloseFont(pGame->pFont); 

    if (pGame->pMusic) closeMusic(pGame->pMusic);
    if (pGame->pSocket) SDLNet_UDP_Close(pGame->pSocket);
    if (pGame->pPacket) SDLNet_FreePacket(pGame->pPacket);

    SDLNet_Quit();
    IMG_Quit();
    SDL_Quit();
}