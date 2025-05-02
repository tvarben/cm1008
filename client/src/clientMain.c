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
#include "menu.h"
#include "ship_data.h"

#define MUSIC_FILEPATH "../lib/resources/music.wav"

typedef struct {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ship *pShips[MAX_PLAYERS];
    int nrOfShips, shipId;
    GameState state;
    Mix_Music *pMusic;
	TTF_Font *pFont, *pSmallFont;
	Text *pStartText;
    Text *pSinglePlayerText, *pGameName, *pExitText, *pPauseText, *pScoreText, *pMultiPlayerText, *pMenuText, *pGameOverText;
    ClientCommand command;
    UDPsocket pSocket;
    IPaddress serverAddress;
    UDPpacket *pPacket;
    bool isRunning;
} Game;

int initiate(Game *pGame);
void startState(Game *pGame);
void ongoingState(Game *pGame);
void multiplayerState(Game *pGame);
void gameOverState(Game *pGame);
void closeGame(Game *pGame);
void handleInput(SDL_Event* pEvent, ClientCommand command, Game* pGame);
bool connectToServer(Game *pGame);
void receiveDataFromServer();
void updateWithServerData(Game *pGame);

int main(int argc, char** argv) {
    Game game = {0};
    if (!initiate(&game)) {
        closeGame(&game);
        return 1;
    }
    
    while (game.isRunning) {
        switch (game.state) {
            case START:
                startState(&game);
                break;
            case ONGOING:
                ongoingState(&game);
                break;
            case MULTIPLAYER:
                multiplayerState(&game);
                break;
            case GAME_OVER:
                gameOverState(&game);
                break;
            default:
                startState(&game);
                break;
        }
    }

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
    /*if (!(pGame->pSocket = SDLNet_UDP_Open(0))) {
        printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }*/
    /*if (SDLNet_ResolveHost(&(pGame->serverAddress), "127.0.0.1", SERVER_PORT)) {
        printf("SDLNet_ResolveHost(127.0.0.1 2000): %s\n", SDLNet_GetError());
        return 0;
    }*/
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

void startState(Game *pGame) {
    SDL_Event event;
    while (pGame->isRunning && pGame->state == START) {
        int x, y;
        SDL_GetMouseState(&x,&y);
        SDL_Point mousePoint = {x,y};
        const SDL_Rect *singleRect = getTextRect(pGame->pSinglePlayerText);
        const SDL_Rect *exitRect = getTextRect(pGame->pExitText);       //Hämta position för rect för Exit-texten
        const SDL_Rect *multiRect = getTextRect(pGame->pMultiPlayerText);
        //const SDL_Rect *gameRect = getTextRect(pGame->pGameName);  
        
        if (SDL_PointInRect(&mousePoint, singleRect)) {
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
            if (event.type == SDL_QUIT || (event.type == SDL_MOUSEBUTTONDOWN && SDL_PointInRect(&mousePoint, exitRect))) {
                pGame->isRunning = false;
                return;
            }else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if(SDL_PointInRect(&mousePoint, singleRect)) {
                    printf("Singleplayer chosen.\n");
                }else if(SDL_PointInRect(&mousePoint, multiRect)) {
                    printf("Multiplayer chosen.\n");
                        /*if(connectToServer(pGame)) {
                            //printf("Connected to server");
                            pGame->state = ONGOING;
                        }*/
                    pGame->state = MULTIPLAYER;
                    return;
                }
            }
        }

        SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 0);
        SDL_RenderClear(pGame->pRenderer);
        SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
        drawText(pGame->pSinglePlayerText);
        drawText(pGame->pMultiPlayerText);
        drawText(pGame->pExitText);
        //drawStars(pGame->pStars,pGame->pRenderer);
        drawText(pGame->pGameName);
        SDL_RenderPresent(pGame->pRenderer);
        SDL_Delay(8);
    }
}

void ongoingState(Game *pGame) {
    SDL_Event event;
    ClientData cData;
    
    while (pGame->isRunning && pGame->state == ONGOING) {
        while (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
            updateWithServerData(pGame);
            //printf("Update with server data.\n");
        }
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pGame->isRunning = false;
                return;
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
    }
}

void multiplayerState(Game *pGame) {
    SDL_Event event;
    bool socketOpened = false, textFieldFocused = false;

    while (pGame->isRunning && pGame->state == MULTIPLAYER) {
        SDL_StartTextInput(); // Enable text input
        static char enteredIPAddress[32] = ""; // Buffer to store the entered string

        while (pGame->isRunning && pGame->state == MULTIPLAYER) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    pGame->isRunning = false;
                    return;
                } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                    SDL_Point click = { event.button.x, event.button.y };
                    SDL_Rect inputBox = { 300, 300, 600, 100 };
                    if (SDL_PointInRect(&click, &inputBox)) {
                        textFieldFocused = true;
                    } else {
                        textFieldFocused = false;
                    }
                } else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(enteredIPAddress) > 0) {
                        enteredIPAddress[strlen(enteredIPAddress) - 1] = '\0'; // Remove the last character
                    } else if (event.key.keysym.sym == SDLK_RETURN) {
                        printf("Entered IP: %s\n", enteredIPAddress);

                        // Attempt to resolve the entered IP address
                        if (SDLNet_ResolveHost(&(pGame->serverAddress), enteredIPAddress, SERVER_PORT) == 0) {
                            printf("Resolved IP: %s\n", enteredIPAddress);
                            if (!socketOpened) {
                                if (!(pGame->pSocket = SDLNet_UDP_Open(0))) {
                                    printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
                                    return;
                                }
                                socketOpened = true;
                            }
                            pGame->pPacket->address.host = pGame->serverAddress.host;
                            pGame->pPacket->address.port = pGame->serverAddress.port;
                            // Attempt to connect to the server
                            if (connectToServer(pGame)) {
                                printf("Connected to server.\n");
                                pGame->state = ONGOING;
                                return;
                            } else {
                                printf("Failed to connect to server.\n");
                                pGame->state = START;
                                return;
                            }
                        } else {
                            printf("Failed to resolve IP: %s\n", enteredIPAddress);
                            pGame->state = START;
                            return;
                        }
                        return;
                    } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        pGame->state = START;
                        return;
                    }
                } else if (event.type == SDL_TEXTINPUT) {
                    if (strlen(enteredIPAddress) < 31) {
                        strncat(enteredIPAddress, event.text.text, sizeof(enteredIPAddress) - strlen(enteredIPAddress) - 1); // Append the entered character
                    }
                }
            }
            // Render the text box
            SDL_SetRenderDrawColor(pGame->pRenderer, 20, 20, 20, 255); // Dark background
            SDL_RenderClear(pGame->pRenderer);

            // Draw the input box
            SDL_Rect box = {300, 300, 600, 100};
            SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255); // Black box
            SDL_RenderFillRect(pGame->pRenderer, &box);
            SDL_SetRenderDrawColor(pGame->pRenderer, 255, 255, 255, 255); // White border
            SDL_RenderDrawRect(pGame->pRenderer, &box);

            // Render the entered text
            SDL_Color color = {255, 255, 255};
            SDL_Rect textRect = { box.x + 5, box.y +10, 0, 0 };
            SDL_Surface* textSurface = TTF_RenderText_Solid(pGame->pSmallFont, enteredIPAddress, color);

            if (textSurface) {
                textRect.w = textSurface->w;
                textRect.h = textSurface->h;

                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, textSurface);
                SDL_RenderCopy(pGame->pRenderer, textTexture, NULL, &textRect);
                SDL_FreeSurface(textSurface);
                SDL_DestroyTexture(textTexture);
            } else {
                textRect.h = TTF_FontHeight(pGame->pSmallFont);
            }

            // Render the prompt text
            SDL_Surface* promptSurface1 = TTF_RenderText_Solid(pGame->pSmallFont, "Type in server IP ADDRESS and press ENTER", color);
            if (promptSurface1) {
                SDL_Texture* promptTexture1 = SDL_CreateTextureFromSurface(pGame->pRenderer, promptSurface1);
                SDL_Rect promptRect1 = {box.x - 150, box.y - 150, promptSurface1->w, promptSurface1->h}; // Position above the input box
                SDL_RenderCopy(pGame->pRenderer, promptTexture1, NULL, &promptRect1);
                SDL_FreeSurface(promptSurface1);
                SDL_DestroyTexture(promptTexture1);
            }

            SDL_Surface* promptSurface2 = TTF_RenderText_Solid(pGame->pSmallFont, "Press ESCAPE to go to the MAIN MENU", color);
            if (promptSurface2) {
                SDL_Texture* promptTexture2 = SDL_CreateTextureFromSurface(pGame->pRenderer, promptSurface2);
                SDL_Rect promptRect2 = {box.x -150, box.y + 150, promptSurface2->w, promptSurface2->h}; // Position below the first line
                SDL_RenderCopy(pGame->pRenderer, promptTexture2, NULL, &promptRect2);
                SDL_FreeSurface(promptSurface2);
                SDL_DestroyTexture(promptTexture2);
            }

            static Uint32 lastToggleTime = 0;
            static bool showCaret = true;

            Uint32 currentTime = SDL_GetTicks();
            if (currentTime > lastToggleTime + 500) {
                showCaret = !showCaret;
                lastToggleTime = currentTime;
            }

            if (textFieldFocused && showCaret) {
                int caretX = textRect.x + textRect.w + 2;
                int caretHeight = 60;
                int caretY = box.y + (box.h - caretHeight) / 2;

                SDL_SetRenderDrawColor(pGame->pRenderer, 255, 255, 255, 255);
                SDL_Rect caretRect = { caretX, caretY, 3, caretHeight };
                SDL_RenderFillRect(pGame->pRenderer, &caretRect);
            }

            SDL_RenderPresent(pGame->pRenderer);
            SDL_Delay(16);
        }
        SDL_StopTextInput();
    }
}

void gameOverState(Game *pGame) {
    while (pGame->isRunning && pGame->state == GAME_OVER) {
        pGame->state = START;
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
    //memcpy(pGame->pPacket->data, "TRYING TO CONNECT", sizeof("TRYING TO CONNECT"));
    //pGame->pPacket->len = sizeof("TRYING TO CONNECT");
    const char *msg = "TRYING TO CONNECT";
    memcpy(pGame->pPacket->data, msg, strlen(msg) + 1);
    pGame->pPacket->len = strlen(msg) + 1;
    pGame->pPacket->address = pGame->serverAddress;
    SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);

    if (SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket) == 0) {
        printf("Failed to send response: %s\n", SDLNet_GetError());
    } else {
        printf("Sent response to client.\n");
    }

    bool connected = false;
    Uint32 startTime = SDL_GetTicks(); // Get the current time in milliseconds
    const Uint32 timeout = 5000;       // Set a timeout of 5000ms (5 seconds)
    while (!connected) {
        if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
            printf("*Received from server: %s\n", (char*)pGame->pPacket->data);
                
            ServerData serverData;/////// test
            memcpy(&serverData, pGame->pPacket->data, sizeof(ServerData));
            pGame->shipId= serverData.sDPlayerId;////// test
            connected = true;
        }
        if (SDL_GetTicks() - startTime > timeout) {
            printf("Connection timed out.\n");
            break;
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