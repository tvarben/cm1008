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
    IPaddress clients[MAX_PLAYERS];
    int nrOfClients;
    UDPsocket pSocket;
    //IPaddress serverAddress;
    UDPpacket *pPacket;

    ServerData serverData;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void closeGame(Game *pGame);
int getClientIndex(Game *pGame, IPaddress *clientAddr);
void sendServerData(Game* pGame);

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

    pGame->pWindow = SDL_CreateWindow("Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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

    //pGame->pPacket->address.host = pGame->serverAddress.host;
    //pGame->pPacket->address.port = pGame->serverAddress.port;

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
    printf("Server is listening on port 2000...\n");
    SDL_Event event;
    ClientData cData;
    
    while (isRunning) {
        switch(pGame->state) {
            case ONGOING:
                //sendServerData(pGame);
                if (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        isRunning = false;
                    }
                }
                while(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)==1) { ///
                    memcpy(&cData, pGame->pPacket->data, sizeof(ClientData));
                
                    int clientIndex = getClientIndex(pGame, &pGame->pPacket->address);
                    if (clientIndex >= 0 && clientIndex < MAX_PLAYERS) {
                        applyShipCommand(pGame->pShips[clientIndex], cData.command);
                        for (int i = 0; i < pGame->nrOfClients; i++) {
                            Uint32 ip = SDL_SwapBE32(pGame->clients[i].host);
                            Uint16 port = SDL_SwapBE16(pGame->clients[i].port);

                            printf("Client %d: %d.%d.%d.%d:%d\n", i,
                            (ip >> 24) & 0xFF,
                            (ip >> 16) & 0xFF,
                            (ip >> 8) & 0xFF,
                            ip & 0xFF,
                            port);
                        }
                    }
                }
                for(int i = 0; i < MAX_PLAYERS; i++) {
                    if (pGame->pShips[i]) {
                        updateShipVelocity(pGame->pShips[i]);
                        updateShip(pGame->pShips[i]);
                    }
                }
                //updateShipVelocity(pGame->pShip);
                //updateShip(pGame->pShip);
                SDL_SetRenderDrawColor(pGame->pRenderer, 30, 30, 30, 255);
                SDL_RenderClear(pGame->pRenderer);   
                for(int i=0; i<MAX_PLAYERS; i++){
                    drawShip(pGame->pShips[i]);
                }
                SDL_RenderPresent(pGame->pRenderer);
                break;
            
            case START:
                pGame->state = ONGOING;
                break;
            case GAME_OVER:
                pGame->state = START;
                break;
            }
        }
        SDL_Delay(8);
}


void sendServerData(Game* pGame) {
    pGame->serverData.gState = pGame->state;

    for(int i=0 ; i<MAX_PLAYERS ; i++){
        getShipDataPackage(pGame->pShips[i], &pGame->serverData.ships[i]);
    }
    for(int i=0 ; i< MAX_PLAYERS ; i++){
        pGame->serverData.playerId =i;
        memcpy(pGame->pPacket->data, &(pGame->serverData), sizeof(ServerData));

        pGame->pPacket->len = sizeof(ServerData);
        pGame->pPacket->address = pGame->clients[i];
        SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
    }

}

int getClientIndex(Game *pGame, IPaddress *clientAddr) {
    for (int i = 0; i < pGame->nrOfClients; i++) {
        if (pGame->clients[i].host == clientAddr->host &&
            pGame->clients[i].port == clientAddr->port) {
            return i; // Existing client
        }
    }
    if (pGame->nrOfClients < MAX_PLAYERS) {
        // New client
        pGame->clients[pGame->nrOfClients] = *clientAddr;
        return pGame->nrOfClients++;
    }
    return -1; // Too many clients
}

void closeGame(Game *pGame) {
    for (int i = 0; i < MAX_PLAYERS; i++) if (pGame->pShips[i]) destroyShip(pGame->pShips[i]);
    //if (pGame->pShip) destroyShip(pGame->pShip);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);

    if (pGame->pStartText) destroyText(pGame->pStartText);
    if (pGame->pGameName) destroyText(pGame->pGameName);
    if (pGame->pExitText) destroyText(pGame->pExitText);
    if (pGame->pFont) TTF_CloseFont(pGame->pFont); 

    if (pGame->pMusic) closeMusic(pGame->pMusic);
    if (pGame->pSocket) SDLNet_UDP_Close(pGame->pSocket);
    if (pGame->pPacket) SDLNet_FreePacket(pGame->pPacket);

    SDLNet_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}