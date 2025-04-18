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

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define MUSIC_FILEPATH "../lib/resources/music.wav"

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

    pGame->pWindow = SDL_CreateWindow("",
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
    SDL_Event event;

    //playMusic(pGame->pMusic, -1);

    while (isRunning) {
        if(pGame->state == ONGOING) {
            while(SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    isRunning = false;
                } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                    handleShipEvent(pGame->pShip, &event);  // track which keys are pressed
                }
            }
        
        
        }else if(pGame->state == START) {
            int x, y;
            SDL_GetMouseState(&x,&y);
            SDL_Point mousePoint = {x, y};                                // Get mouse position

            const SDL_Rect *startRect = getTextRect(pGame->pStartText);   // Get rect for Start text
            const SDL_Rect *exitRect = getTextRect(pGame->pExitText);     // Get rect for Exit text 
            
            if(SDL_PointInRect(&mousePoint, startRect)) {
                setTextColor(pGame->pStartText, 255, 255, 100, pGame->pFont, "Start");
            } else {
                setTextColor(pGame->pStartText, 238, 168, 65, pGame->pFont, "Start");
            }
            if(SDL_PointInRect(&mousePoint, exitRect)) {
                setTextColor(pGame->pExitText, 255, 100, 100, pGame->pFont, "Exit");
            } else {
                setTextColor(pGame->pExitText, 238, 168, 65, pGame->pFont, "Exit");
            }

            while(SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    isRunning = false;
                } else if (SDL_PointInRect(&mousePoint, startRect) && event.type == SDL_MOUSEBUTTONDOWN) {
                    
                    int nrOfAttempts = 0, maxAttempts = 10;
                    bool ServerResponded = false;                               //// reset flags before trying to connect to server
                    
                    while (!ServerResponded && nrOfAttempts < maxAttempts) {
                                                                                // Send connection request
                        //sprintf((char*)pGame->pPacket->data, "Client %d is trying to connect!\n", pGame->pPacket->address.port); // Client is sending random port numbers. So let the server be the one to assign the port number.
                        strcpy((char*)pGame->pPacket->data, "JOIN_REQUEST");
                        pGame->pPacket->len = strlen((char*)pGame->pPacket->data) + 1;
                        SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
                    
                        
                        SDL_Delay(100); // Wait 100ms to receive response from server                    
                                                                    
                                        // server responded? if so, exit the loop.
                        if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
                            printf("\nServer responded: %s\n", (char*)pGame->pPacket->data);
                            ServerResponded = true;
                        } else {
                            nrOfAttempts++;
                            printf("Attempt %d: Server isn't responding...\n", nrOfAttempts);
                        }
                    }
                    
                    if (ServerResponded) {       // Server responded, start the game. otherwise, return to main menu to try again.
                        pGame->state = ONGOING;
                    } else {
                        printf("Error... Server didn't respond after %d attempts. Returning to Main Menu.\n", maxAttempts);
                        pGame->state = START;
                    } 

                } else if (SDL_PointInRect(&mousePoint, exitRect) && event.type == SDL_MOUSEBUTTONDOWN) {
                    isRunning = false;
                }
            }
        

            /*while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                strcpy((char*)pGame->pPacket->data, "Hej pa dig!");
                pGame->pPacket->len = strlen((char*)pGame->pPacket->data) + 1;
                SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);

                if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
                    printf("Server recieved a command from %x:%d: %s\n",
                                                pGame->pPacket->address.host, 
                                                pGame->pPacket->address.port,
                                                (char*)pGame->pPacket->data);
                }
                //printf("Sent: %s\n", (char*)pGame->pPacket->data);
                
                //pGame->pPacket->len = sizeof(ClientData);

            } else if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_2){
                isRunning = false;
            }
            if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
                printf("Received from server: %s\n", (char*)pGame->pPacket->data);
            }
        }*/
        }

        SDL_SetRenderDrawColor(pGame->pRenderer, 30, 30, 30, 255);
        SDL_RenderClear(pGame->pRenderer);              //Clear the first frame when the game starts,
                                                        //otherwise flickering issues on mac/linux
        
        drawText(pGame->pStartText);
        drawText(pGame->pExitText);
        drawText(pGame->pGameName);
        SDL_RenderPresent(pGame->pRenderer);

        
    }
}


void closeGame(Game *pGame) {
    if (pGame->pShip) destroyShip(pGame->pShip);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);

    if(pGame->pStartText) destroyText(pGame->pStartText);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont); 

    closeMusic(pGame->pMusic);

    SDLNet_Quit();
    IMG_Quit();
    SDL_Quit();
}