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
#include "ship_data.h"

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 400
#define MUSIC_FILEPATH "../lib/resources/music.wav"

typedef enum  { START, ONGOING, GAME_OVER 
} GameState;

typedef struct {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ship *pShips[MAX_PLAYERS];
    int nrOfShips;
    //Ship *pShip;
    GameState state;
    Mix_Music *pMusic;
	TTF_Font *pFont;
	Text *pStartText, *pGameName, *pExitText;

    ClientCommand command;

    UDPsocket pSocket;
    IPaddress serverAddress;
    UDPpacket *pPacket;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void closeGame(Game *pGame);
void handleInput(SDL_Event* pEvent, ClientCommand command, Game* pGame);

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

    pGame->pWindow = SDL_CreateWindow("Client",
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

	pGame->pFont = TTF_OpenFont("../lib/resources/arial.ttf", 100);
    if(!pGame->pFont ) {
        printf("Error: %s\n",TTF_GetError());
        return 0;
    }

	pGame->pStartText = createText(pGame->pRenderer,238,168,65,pGame->pFont,"Start [1]",WINDOW_WIDTH/3,WINDOW_HEIGHT/2+100);
    pGame->pGameName = createText(pGame->pRenderer,238,168,65,pGame->pFont,"SpaceShooter",WINDOW_WIDTH/2,WINDOW_HEIGHT/4);
    pGame->pExitText = createText(pGame->pRenderer,238,168,65,pGame->pFont,"Exit [2]",WINDOW_WIDTH/1.5,WINDOW_HEIGHT/2+100);
    if(!pGame->pFont){
        printf("Error: %s\n",TTF_GetError());
        return 0;
    } 
    if (!(pGame->pSocket = SDLNet_UDP_Open(0))) {
        printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }
    if (SDLNet_ResolveHost(&(pGame->serverAddress), "127.0.0.1", 2000)) {
        printf("SDLNet_ResolveHost(127.0.0.1 2000): %s\n", SDLNet_GetError());
        return 0;
    }
    if (!(pGame->pPacket = SDLNet_AllocPacket(512))) {
        printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 0;
    }

    pGame->pPacket->address.host = pGame->serverAddress.host;
    pGame->pPacket->address.port = pGame->serverAddress.port;

    //pGame->pShip = createShip(pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    for(int i = 0; i < MAX_PLAYERS; i++){
        pGame->pShips[i] = createShip(i, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    pGame->nrOfShips = MAX_PLAYERS;

    for(int i = 0; i < MAX_PLAYERS; i++){
        if (!pGame->pShips[i]) {
            printf("Error: %s\n", SDL_GetError());
            closeGame(pGame);
            return 0;
        }
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
    SDL_Event event;
    ClientData cData;
    for(int i = 0; i < MAX_PLAYERS; i++) {
        resetShip(pGame->pShips[i]);
    }
    //playMusic(pGame->pMusic, -1);

    while (isRunning) {
    
        switch (pGame->state) {
            case ONGOING:
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        isRunning = false;
                    } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                        handleInput(&event, cData.command, pGame);
                    }

                    /*for(int i = 0; i < pGame->nrOfShips; i++){
                            drawShip(pGame->pShips[i]);
                    }*/


                    if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
                        printf("Received from server: %s\n", (char*)pGame->pPacket->data);
                    }

                    SDL_Delay(8);  // Delay to limit the frame rate
                }
                
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    if (pGame->pShips[i]) {
                        updateShipVelocity(pGame->pShips[i]);
                        updateShip(pGame->pShips[i]);
                    }
                }
                
                //updateShipVelocity(pGame->pShips[0]);
                //updateShip(pGame->pShips[0]);
                SDL_SetRenderDrawColor(pGame->pRenderer, 30, 30, 30, 255);
                SDL_RenderClear(pGame->pRenderer);
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    drawShip(pGame->pShips[i]);   
                }
                SDL_RenderPresent(pGame->pRenderer);
                break;
            case START:
                pGame->state = ONGOING;
                break;
        }
    }
}

void handleInput(SDL_Event* pEvent, ClientCommand command, Game* pGame) {
    ClientData cData;
    SDL_Scancode key = pEvent->key.keysym.scancode;
    cData.playerId = 1;
    bool pressed = false;
    if (pEvent->type == SDL_KEYDOWN || pEvent->type == SDL_KEYUP) {
        switch(key) {
            case SDL_SCANCODE_UP:
                cData.command = pEvent->type == SDL_KEYDOWN ? MOVE_UP : STOP_SHIP;
                handleShipEvent(pGame->pShips[0], pEvent);
                printf("MOVE_UP SENT!\n");
                break;
            case SDL_SCANCODE_DOWN:
                cData.command = pEvent->type == SDL_KEYDOWN ? MOVE_DOWN : STOP_SHIP;
                handleShipEvent(pGame->pShips[0], pEvent);
                printf("MOVE_DOWN SENT!\n");
                break;
            case SDL_SCANCODE_LEFT:
                cData.command = pEvent->type == SDL_KEYDOWN ? MOVE_LEFT : STOP_SHIP;
                handleShipEvent(pGame->pShips[0], pEvent);
                printf("MOVE_LEFT SENT!\n");
                break;
            case SDL_SCANCODE_RIGHT:
                cData.command = pEvent->type == SDL_KEYDOWN ? MOVE_RIGHT : STOP_SHIP;
                handleShipEvent(pGame->pShips[0], pEvent);
                printf("MOVE_RIGHT SENT!\n");
                break;
            default:
                cData.command = STOP_SHIP;
                break;
        }
    }

    memcpy(pGame->pPacket->data, &cData, sizeof(ClientData));
    pGame->pPacket->len = sizeof(ClientData);
    SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
}

void closeGame(Game *pGame) {
    for (int i = 0; i < MAX_PLAYERS;i++) if (pGame->pShips[i]) destroyShip(pGame->pShips[i]);
    //if (pGame->pShip) destroyShip(pGame->pShip);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);

    if(pGame->pStartText) destroyText(pGame->pStartText);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont); 

    closeMusic(pGame->pMusic);

    SDLNet_Quit();
    IMG_Quit();
    SDL_Quit();
}