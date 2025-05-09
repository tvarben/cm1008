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
#include "enemy_1.h"
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

    int nrOfEnemiesToSpawn_1;
    EnemyImage *pEnemyImage;
    Enemy *pEnemies[MAX_ENEMIES];
    int nrOfEnemies_1;
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
void resetEnemy(Game *pGame);
void spawnEnemies(Game *pGame, int amount);
void updateEnemies(Game *pGame, int *amount);
bool areTheyAllDead(Game *pGame);
void getNrOfEnemiesDataPackage(int nrOfEnemies_1, ServerData *pServerData);

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
    if (!(pGame->pPacket = SDLNet_AllocPacket(2048))) {
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

    pGame->pEnemyImage = initiateEnemy(pGame->pRenderer);
    pGame->nrOfEnemies_1 = 0;
    printf("Enemy image created\n");

    pGame->isRunning = true;
    pGame->state = START;
    return 1;
}

void run(Game *pGame) {
    pGame->nrOfEnemiesToSpawn_1= WAVE_1_EASY_MAP; //Change it in ship_data.h. Initial number of enemies, they get incremented by 2 down below in updateEnemies()
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
                    printf("Server is listening on port %hu...\n", SERVER_PORT);
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
    printf("Ongoing\n");

    pGame->nrOfEnemies_1 = 0;
    
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
        
        while (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
            printf("Packets succesfully recieved\n");
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
            updateEnemies(pGame, &pGame->nrOfEnemiesToSpawn_1); // Calls spawnEnemy() down at the bottom of the code
            for (int i = 0; i < pGame->nrOfEnemies_1 && i < MAX_ENEMIES; i++) {
                updateEnemy(pGame->pEnemies[i]);                // locally
            }
            SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
            SDL_RenderClear(pGame->pRenderer);
            for(int i=0; i<MAX_PLAYERS; i++){
                drawShip(pGame->pShips[i]);
            }
            for (int i = 0; i < pGame->nrOfEnemies_1 && i < MAX_ENEMIES; i++) {
                if (isEnemyActive(pGame->pEnemies[i]) == true) {    //locally
                    drawEnemy(pGame->pEnemies[i]);
                    //printf("ALIVE or DEAD? %d\n", isEnemyActive(pGame->pEnemies[i]));
                }
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
            if (event.type == SDL_QUIT){
                pGame->isRunning = false;
                return;
            }
        }
        if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket) == 1) {
            addClient(pGame);
            printf("After addClient()\n");
            if (pGame->nrOfClients == MAX_PLAYERS) {
                pGame->state = ONGOING;
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    const char *msg = "ONGOING";
                    memcpy(pGame->pPacket->data, msg, strlen(msg)+1);
                    pGame->pPacket->address= pGame->clients[i];
                    SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
                }
            }
            printf("Sent message 'ongoing' to all clients\n");
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
            printf("Sending packet data (%d bytes): \"%.*s\"\n",pGame->pPacket->len,pGame->pPacket->len,(char*)pGame->pPacket->data);
            if (SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket)){
                printf("Sent connection confirmation to client %d. \n", clientIndex);
                ServerData *sd = &pGame->serverData;
                printf("*Sending first packet to client %d:\n", clientIndex);
                printf("  gState: %d\n", sd->gState);
                printf("  sDPlayerId (assigned ID): %d\n", sd->sDPlayerId);
                // Print each connected player's ship data
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    printf("  Ship %d: x = %.2f, y = %.2f, angle = %.2f, alive = %d\n",
                        i, sd->ships[i].x, sd->ships[i].y, sd->ships[i].vx, sd->ships[i].vy);
                }
                // Print enemy count
                printf("  Enemies_1: %d active, %d to spawn\n", sd->nrOfEnemies_1, sd->nrOfEnemiesToSpawn_1);
                // Print each enemy's data
                for (int i = 0; i < sd->nrOfEnemies_1; i++) {
                    printf("    Enemy_1[%d]: x = %.2f, y = %.2f, hp = %d, alive = %d\n",
                        i, sd->enemies_1[i].x, sd->enemies_1[i].y, sd->enemies_1[i].active);
                }
            }else 
                printf("Failed to Send connection confirmation to client %d.\n", clientIndex);
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
    
    for(int i = 0; i < pGame->nrOfEnemies_1 && i < MAX_ENEMIES; i++)
        getEnemy_1_DataPackage(pGame->pEnemies[i], &pGame->serverData.enemies_1[i]);
    pGame->serverData.nrOfEnemies_1 = pGame->nrOfEnemies_1;
    
        
    for(int i=0 ; i< MAX_PLAYERS ; i++){
        pGame->serverData.sDPlayerId =i;
        memcpy(pGame->pPacket->data, &(pGame->serverData), sizeof(ServerData));
        pGame->pPacket->len = sizeof(ServerData);
        pGame->pPacket->address = pGame->clients[i];
        SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);

        /*ServerData *sd = &pGame->serverData;
        printf("*Sending updated packet to client %d:\n", i);
        printf("gState: %d\n", sd->gState);
        printf("  sDPlayerId (assigned ID): %d\n", sd->sDPlayerId);
        
        // Print each connected player's ship data
        for (int i = 0; i < MAX_PLAYERS; i++) {
            printf("  Ship %d: x = %.2f, y = %.2f, angle = %.2f, alive = %d\n",
                i, sd->ships[i].x, sd->ships[i].y, sd->ships[i].vx, sd->ships[i].vy);
        }
        // Print enemy count
        printf("  Enemies_1: %d active, %d to spawn\n", sd->nrOfEnemies_1, sd->nrOfEnemiesToSpawn_1);
        // Print each enemy's data
        for (int i = 0; i < sd->nrOfEnemies_1; i++) {
            printf("    Enemy_1[%d]: x = %.2f, y = %.2f, alive = %d\n",
                i, sd->enemies_1[i].x, sd->enemies_1[i].y, sd->enemies_1[i].active);
        }*/
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
        for (int i = 0; i < pGame->nrOfClients; i++) {
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

void resetEnemy(Game *pGame) {
    for (int i = 0; i < pGame->nrOfEnemies_1 && i < MAX_ENEMIES; i++) {
      destroyEnemy(pGame->pEnemies[i]);
    }
    pGame->nrOfEnemies_1 = 0;
    printf("Enemies destroyed\n");
    // add for new enemy here
}

  void spawnEnemies(Game *pGame, int amount) {
    for (int i = 0; i < amount; i++) {
        pGame->pEnemies[pGame->nrOfEnemies_1] =createEnemy(pGame->pEnemyImage, WINDOW_WIDTH, WINDOW_HEIGHT);
          pGame->nrOfEnemies_1++;
    }
}

void updateEnemies(Game *pGame, int *amount) {
    if (areTheyAllDead(pGame) == true)
    {
      (*amount) += 2; // increments even for first wave. WHY?
      if ((*amount) > MAX_ENEMIES)
        (*amount) = MAX_ENEMIES;
      pGame->nrOfEnemies_1 = 0;
      spawnEnemies(pGame, *amount);
    }
}

bool areTheyAllDead(Game *pGame) {
    for (int i = 0; i < pGame->nrOfEnemies_1 && i < MAX_ENEMIES; i++) {
      if (isEnemyActive(pGame->pEnemies[i]) == true) {
        return false;
      }
    }
    return true;
}

