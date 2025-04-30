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
    int nrOfShips, shipId;
    //Ship *pShip;
    GameState state;
    Mix_Music *pMusic;
	TTF_Font *pFont, *pSmallFont;
	Text *pStartText;
    Text *pSinglePlayerText, *pGameName, *pExitText, *pPauseText, *pScoreText, *pMultiPlayerText, *pMenuText, *pGameOverText;


    ClientCommand command;

    UDPsocket pSocket;
    IPaddress serverAddress;
    UDPpacket *pPacket;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void closeGame(Game *pGame);
void handleInput(SDL_Event* pEvent, ClientCommand command, Game* pGame);
bool connectToServer(Game *pGame);
void receiveDataFromServer();
void updateWithServerData(Game *pGame);

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
    pGame->pSmallFont = TTF_OpenFont("../lib/resources/arial.ttf", 50);
    if(!pGame->pFont && !pGame->pSmallFont) {
        printf("Error: %s\n",TTF_GetError());
        return 0;
    }

    pGame->pSinglePlayerText = createText(pGame->pRenderer,255,0,0,pGame->pSmallFont,"Singleplayer",WINDOW_WIDTH/2, 330);
    pGame->pMultiPlayerText = createText(pGame->pRenderer,255,0,0,pGame->pSmallFont,"Multiplayer",WINDOW_WIDTH/2, 450);
    pGame->pGameName = createText(pGame->pRenderer,255,0,0,pGame->pFont,"SpaceShooter",WINDOW_WIDTH/2,WINDOW_HEIGHT/8);
    pGame->pExitText = createText(pGame->pRenderer,255,0,0,pGame->pSmallFont,"Exit",WINDOW_WIDTH/2, 570);

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
    int x, y;
    //playMusic(pGame->pMusic, -1);
    //const SDL_Rect *startRect = getTextRect(pGame->pSinglePlayerText);
    //const SDL_Rect *exitRect = getTextRect(pGame->pExitText);       //Hämta position för rect för Exit-texten
    //const SDL_Rect *multiRect = getTextRect(pGame->pMultiPlayerText);
    //const SDL_Rect *menuRect = getTextRect(pGame->pMenuText);
    //const SDL_Rect *gameRect = getTextRect(pGame->pGameName);

    while (isRunning) {
        switch (pGame->state) {
            case ONGOING:
                while (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
                    updateWithServerData(pGame);
                    //printf("Update with server data.\n");
                }

                if (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        isRunning = false;
                    } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                        handleInput(&event, cData.command, pGame);
                    }
                }
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    if (pGame->pShips[i]) {
                        updateShipVelocity(pGame->pShips[i]);
                        updateShipOnClients(pGame->pShips[i], i, pGame->shipId); // <--- pass remote shipId and myShipId
                    }
                }
                SDL_SetRenderDrawColor(pGame->pRenderer, 30, 30, 30, 255);
                SDL_RenderClear(pGame->pRenderer);
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    drawShip(pGame->pShips[i]);   
                }
                SDL_RenderPresent(pGame->pRenderer);
                break;
            case START:
                SDL_GetMouseState(&x,&y);
                SDL_Point mousePoint = {x,y};
                const SDL_Rect *startRect = getTextRect(pGame->pSinglePlayerText);
                const SDL_Rect *exitRect = getTextRect(pGame->pExitText);       //Hämta position för rect för Exit-texten
                const SDL_Rect *multiRect = getTextRect(pGame->pMultiPlayerText);
                const SDL_Rect *gameRect = getTextRect(pGame->pGameName);
            
                if (SDL_PointInRect(&mousePoint, startRect)) {
                    setTextColor(pGame->pSinglePlayerText, 255, 255, 255, pGame->pSmallFont, "Singleplayer");
                } else {
                    setTextColor(pGame->pSinglePlayerText, 255, 0, 0, pGame->pSmallFont, "Singleplayer");
                }
                if (SDL_PointInRect(&mousePoint, multiRect)) {
                    setTextColor(pGame->pMultiPlayerText, 255, 255, 255, pGame->pSmallFont, "Multiplayer");
                } else {
                    setTextColor(pGame->pMultiPlayerText, 255, 0, 0, pGame->pSmallFont, "Multiplayer");
                }
                if (SDL_PointInRect(&mousePoint, exitRect)) {
                    setTextColor(pGame->pExitText, 255, 255, 255, pGame->pSmallFont, "Exit");
                } else {
                    setTextColor(pGame->pExitText, 255, 0, 0, pGame->pSmallFont, "Exit");
                }
            
                if (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        isRunning = false;
                    } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                        if(connectToServer(pGame)) {
                            //printf("Connected to server");
                            pGame->state = ONGOING;
                        }
                    }
                }
                SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
                SDL_RenderClear(pGame->pRenderer);
                SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
                drawText(pGame->pSinglePlayerText);
                drawText(pGame->pMultiPlayerText);
                drawText(pGame->pExitText);
                //drawStars(pGame->pStars,pGame->pRenderer);
                drawText(pGame->pGameName);
                SDL_RenderPresent(pGame->pRenderer);
                break;
        }
    }
}

void updateWithServerData(Game *pGame) {
    ServerData serverData;/////// test
    memcpy(&serverData, pGame->pPacket->data, sizeof(ServerData));
    pGame->shipId= serverData.sDPlayerId;////// test
    for(int i = 0; i < MAX_PLAYERS; i++) {
        if (pGame->pShips[i])
            updateShipsWithServerData(pGame->pShips[i], &serverData.ships[i], i, pGame->shipId);
    }
}

bool connectToServer(Game *pGame) {
    memcpy(pGame->pPacket->data, "TRYING TO CONNECT", sizeof("TRYING TO CONNECT"));
    pGame->pPacket->len = sizeof("TRYING TO CONNECT");
    SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);

    bool connected = false;
    while (!connected) {
        if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
            printf("*Received from server: %s\n", (char*)pGame->pPacket->data);
                
            ServerData serverData;/////// test
            memcpy(&serverData, pGame->pPacket->data, sizeof(ServerData));
            pGame->shipId= serverData.sDPlayerId;////// test
            connected = true;
        }
    }
    return connected;
}

void receiveDataFromServer() {
    printf("receiveDataFromServer().\n");
}

void handleInput(SDL_Event* pEvent, ClientCommand command, Game* pGame) {
    ClientData cData;
    cData.cDPlayerId = pGame->shipId;
    SDL_Scancode key = pEvent->key.keysym.scancode;
    if (pEvent->type == SDL_KEYDOWN || pEvent->type == SDL_KEYUP) {
        switch(key) {
            case SDL_SCANCODE_UP:
                cData.command = pEvent->type == SDL_KEYDOWN ? MOVE_UP : STOP_SHIP;
                applyShipCommand(pGame->pShips[pGame->shipId], cData.command);
                break;
            case SDL_SCANCODE_DOWN:
                cData.command = pEvent->type == SDL_KEYDOWN ? MOVE_DOWN : STOP_SHIP;
                applyShipCommand(pGame->pShips[pGame->shipId], cData.command);
                break;
            case SDL_SCANCODE_LEFT:
                cData.command = pEvent->type == SDL_KEYDOWN ? MOVE_LEFT : STOP_SHIP;
                applyShipCommand(pGame->pShips[pGame->shipId], cData.command);
                break;
            case SDL_SCANCODE_RIGHT:
                cData.command = pEvent->type == SDL_KEYDOWN ? MOVE_RIGHT : STOP_SHIP;
                applyShipCommand(pGame->pShips[pGame->shipId], cData.command);
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
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);

    if (pGame->pGameName) destroyText(pGame->pGameName);
    if (pGame->pSinglePlayerText) destroyText(pGame->pSinglePlayerText);
    if (pGame->pMultiPlayerText) destroyText(pGame->pMultiPlayerText);
    if (pGame->pExitText) destroyText(pGame->pExitText);
    
    if (pGame->pFont) TTF_CloseFont(pGame->pFont); 
    if (pGame->pSmallFont) TTF_CloseFont(pGame->pSmallFont);

    if (pGame->pMusic) closeMusic(pGame->pMusic);
    if (pGame->pSocket) SDLNet_UDP_Close(pGame->pSocket);
    if (pGame->pPacket) SDLNet_FreePacket(pGame->pPacket);

    SDLNet_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}