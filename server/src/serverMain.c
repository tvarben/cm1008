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

#define MUSIC_FILEPATH "../lib/resources/music.wav"

typedef struct {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ship *pShips[MAX_PLAYERS];
    int nrOfShips;
    GameState state;
    Mix_Music *pMusic;
	TTF_Font *pFont;
	Text *pStartText, *pGameName, *pExitText, *pLobbyText;
    ClientCommand command;
    IPaddress clients[MAX_PLAYERS];
    int nrOfClients;
    UDPsocket pSocket;
    UDPpacket *pPacket;
    ServerData serverData;
    bool isRunning;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void handleStartState(Game *pGame);
void handleOngoingState(Game *pGame);
void handleLobbyState(Game *pGame);
void addClient(Game *pGame);
void closeGame(Game *pGame);
int getClientIndex(Game *pGame, IPaddress *clientAddr);
void sendServerData(Game* pGame);

int main(int argc, char** argv) {
    Game game = {0};
    printf("doing intiate\n");
    if (!initiate(&game)){ 
        closeGame(&game); 
        return 1;
    }
    printf("entering run\n");
    run(&game);
    printf("entering close game from main\n");
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
        return 0;
    }
	if(TTF_Init()!=0) {
        printf("Error: %s\n",TTF_GetError());
        return 0;
    }
    if (SDLNet_Init()) {
        printf("SDLNet_Init: %s\n", SDLNet_GetError());
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
	pGame->pStartText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont,
                                "Start [1]", WINDOW_WIDTH/3, WINDOW_HEIGHT/2+100);
    pGame->pGameName = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont,
                                "SpaceShooter", WINDOW_WIDTH/2, WINDOW_HEIGHT/4);
    pGame->pExitText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont,
                                "Exit [2]", WINDOW_WIDTH/1.5, WINDOW_HEIGHT/2+100);
    pGame->pLobbyText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont,
                                "Waiting on clients...", WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    if(!pGame->pFont){
        printf("Error: %s\n",TTF_GetError());
        return 0;
    } 
    if (!(pGame->pSocket = SDLNet_UDP_Open(SERVER_PORT))) {
        printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }
    if (!(pGame->pPacket = SDLNet_AllocPacket(512))) {
        printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 0;
    }
    
    for(int i = 0; i < MAX_PLAYERS; i++){
        pGame->pShips[i] = createShip(i, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    pGame->nrOfShips = MAX_PLAYERS;
    for(int i = 0; i < MAX_PLAYERS; i++){
        if (!pGame->pShips[i]) {
            printf("Error: %s\n", SDL_GetError());
            return 0;
        }
    }
    if (!initMusic(&pGame->pMusic, MUSIC_FILEPATH)) {
        printf("Error: %s\n",Mix_GetError());
        return 0;
    }
    pGame->isRunning = true;
    pGame->state = START;
    return 1;
}

void run(Game *pGame) {
    printf("Server is listening on port %hu...\n", SERVER_PORT);
    
    while (pGame->isRunning) { 
        switch(pGame->state) {
            case START:
                handleStartState(pGame);
                break;
            case ONGOING: //Lägg till getTicks()
                handleOngoingState(pGame);
                break;
            case LOBBY:
                handleLobbyState(pGame);
                break;
            case GAME_OVER:
                pGame->state = START;
                break;
            default:
                pGame->state = START;
                break;
        }
    }
}

void handleStartState(Game *pGame) {
    SDL_Event event;
    while (pGame->isRunning && pGame->state == START) {
        SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pGame->pRenderer);
        drawText(pGame->pGameName);
        drawText(pGame->pStartText);
        drawText(pGame->pExitText);
        SDL_RenderPresent(pGame->pRenderer);

        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pGame->isRunning = false;
                return;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_1) {
                    pGame->state = LOBBY;
                    return;
                } else if (event.key.keysym.sym == SDLK_2) {
                    pGame->isRunning = false;
                    return;
                }
            }
        }
        SDL_Delay(8);
    }
}

void handleOngoingState(Game *pGame) {
    SDL_Event event;
    ClientData cData;
    Uint32 now = 0, delta = 0,lastUpdate = SDL_GetTicks();
    const Uint32 tickInterval = 16;

    while (pGame->isRunning && pGame->state == ONGOING) {
        now = SDL_GetTicks();
        delta = now - lastUpdate;
        int clientIndex;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pGame->isRunning = false;
                return;
            }
        }
        while (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)==1) {
            // Check if it's a connection request
            /*Uint32 ip = SDL_SwapBE32(pGame->pPacket->address.host);
            Uint16 port = SDL_SwapBE16(pGame->pPacket->address.port);
            printf("Received packet from %d.%d.%d.%d:%d\n", (ip >> 24) & 0xFF,
                            (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF, port);*/
            memcpy(&cData, pGame->pPacket->data, sizeof(ClientData));
            clientIndex = getClientIndex(pGame, &pGame->pPacket->address); 
            if (clientIndex >= 0 && clientIndex < MAX_PLAYERS)
                applyShipCommand(pGame->pShips[clientIndex], cData.command);
            }
            
        if (delta >= tickInterval) {
            lastUpdate = now;
            for(int i = 0; i < MAX_PLAYERS; i++) {
                if (pGame->pShips[i]) {
                    updateShipVelocity(pGame->pShips[i]);
                    updateShipOnServer(pGame->pShips[i]);
                }
            }
            SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
            SDL_RenderClear(pGame->pRenderer);
            for(int i=0; i<MAX_PLAYERS; i++){
                drawShip(pGame->pShips[i]);
            }
            SDL_RenderPresent(pGame->pRenderer);
            sendServerData(pGame);
        }
    }
}

void handleLobbyState(Game *pGame) {
    SDL_Event event;
    while (pGame->isRunning && pGame->state == LOBBY) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                pGame->isRunning = false;
                return;
        }
        if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket) == 1) {
            addClient(pGame);
            printf("After addClient()");
            if (pGame->nrOfClients == MAX_PLAYERS) pGame->state = ONGOING;
        }
        SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pGame->pRenderer);
        drawText(pGame->pLobbyText);
        SDL_RenderPresent(pGame->pRenderer);
        SDL_Delay(32);
    }
}

void addClient(Game *pGame) {
    printf("Trying to add client to server.\n");
    // Check if it's a connection request
    /*Uint32 ip = SDL_SwapBE32(pGame->pPacket->address.host);
    Uint16 port = SDL_SwapBE16(pGame->pPacket->address.port);
    printf("Received packet from %d.%d.%d.%d:%d\n", (ip >> 24) & 0xFF,
                        (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF, port);*/
    if (strncmp((char*)pGame->pPacket->data, "TRYING TO CONNECT", 17) == 0) {
        printf("Received connection request.\n");
        int clientIndex = getClientIndex(pGame, &pGame->pPacket->address);
        if (clientIndex >= 0 && clientIndex < MAX_PLAYERS) {
            // Respond with a client number
            pGame->serverData.sDPlayerId = clientIndex;
            memcpy(pGame->pPacket->data, &pGame->serverData, sizeof(ServerData));
            pGame->pPacket->len = sizeof(ServerData);
            pGame->pPacket->address = pGame->clients[clientIndex];
            SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
            printf("Sent connection confirmation to client %d.\n", clientIndex);
        }
    }
}

/*
void handleGameOverState(Game *pGame) {

}*/

void sendServerData(Game* pGame) {
    if (pGame->nrOfClients == 0) return; // No clients connected
    pGame->serverData.gState = pGame->state;
    for(int i=0 ; i<MAX_PLAYERS ; i++)
        getShipDataPackage(pGame->pShips[i], &pGame->serverData.ships[i]);
    for(int i=0 ; i< MAX_PLAYERS ; i++){
        pGame->serverData.sDPlayerId =i;
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
        //pGame->nrOfClients++;
        for (int i = 0; i <= pGame->nrOfClients; i++) {
            Uint32 ip = SDL_SwapBE32(pGame->clients[i].host);
            Uint16 port = SDL_SwapBE16(pGame->clients[i].port);
            printf("Client %d: %d.%d.%d.%d:%d\n", i, (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
                                        (ip >> 8) & 0xFF, ip & 0xFF, port);
        }
        return pGame->nrOfClients++;
    }
    return -1; // Too many clients
}

void closeGame(Game *pGame) {
    for (int i = 0; i < MAX_PLAYERS; i++) if (pGame->pShips[i]) destroyShip(pGame->pShips[i]);
    //if (pGame->pShip) destroyShip(pGame->pShip);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    printf("Renderer destroyed\n");
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    printf("Window\n");

    if (pGame->pStartText) destroyText(pGame->pStartText);
    printf("starttext\n");
    if (pGame->pGameName) destroyText(pGame->pGameName);
    if (pGame->pExitText) destroyText(pGame->pExitText);
    if (pGame->pLobbyText) destroyText(pGame->pLobbyText);
    if (pGame->pFont) TTF_CloseFont(pGame->pFont); 

    if (pGame->pMusic) closeMusic(pGame->pMusic);
    if (pGame->pSocket) SDLNet_UDP_Close(pGame->pSocket);
    printf("socket\n");
    if (pGame->pPacket) SDLNet_FreePacket(pGame->pPacket);
    printf("Freepacket\n");

    SDLNet_Quit();
    printf("SDL_NETQuit()\n");
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    printf("SDL_Quit");
}