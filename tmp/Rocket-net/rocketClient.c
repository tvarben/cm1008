#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>
#include "rocket_data.h"
#include "rocket.h"
#include "stars.h"
#include "text.h"

struct game{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Rocket *pRocket[MAX_ROCKETS];
    int nr_of_rockets, rocketNr;
    Stars *pStars;
    TTF_Font *pFont, *pScoreFont;
    Text *pOverText, *pStartText, *pWaitingText;
    GameState state;

    UDPsocket pSocket;
	IPaddress serverAddress;
	UDPpacket *pPacket;

};
typedef struct game Game;

int initiate(Game *pGame);
void run(Game *pGame);
void close(Game *pGame);
void handleInput(Game *pGame,SDL_Event *pEvent);
void updateWithServerData(Game *pGame);

int main(int argv, char** args){
    Game g={0};
    if(!initiate(&g)) return 1;
    run(&g);
    close(&g);

    return 0;
}

int initiate(Game *pGame){
    srand(time(NULL));
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }
    if(TTF_Init()!=0){
        printf("Error: %s\n",TTF_GetError());
        SDL_Quit();
        return 0;
    }
    if (SDLNet_Init())
	{
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
        TTF_Quit();
        SDL_Quit();
		return 0;
	}


    pGame->pWindow = SDL_CreateWindow("Rocket Client",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH,WINDOW_HEIGHT,0);
    if(!pGame->pWindow){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!pGame->pRenderer){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;    
    }

    pGame->pFont = TTF_OpenFont("../lib/resources/arial.ttf", 100);
    pGame->pScoreFont = TTF_OpenFont("../lib/resources/arial.ttf", 30);
    if(!pGame->pFont || !pGame->pScoreFont){
        printf("Error: %s\n",TTF_GetError());
        close(pGame);
        return 0;
    }

    if (!(pGame->pSocket = SDLNet_UDP_Open(0))) { //0 means not a server
		printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		return 0;
	}
	if (SDLNet_ResolveHost(&(pGame->serverAddress), "127.0.0.1", 2000)){
		printf("SDLNet_ResolveHost(127.0.0.1 2000): %s\n", SDLNet_GetError());
		return 0;
	}
    if (!(pGame->pPacket = SDLNet_AllocPacket(512))){
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		return 0;
	}
    pGame->pPacket->address.host = pGame->serverAddress.host;
    pGame->pPacket->address.port = pGame->serverAddress.port;

    for(int i=0;i<MAX_ROCKETS;i++)
        pGame->pRocket[i] = createRocket(i,pGame->pRenderer,WINDOW_WIDTH,WINDOW_HEIGHT);
    pGame->nr_of_rockets = MAX_ROCKETS;
    pGame->pStars = createStars(WINDOW_WIDTH*WINDOW_HEIGHT/10000,WINDOW_WIDTH,WINDOW_HEIGHT);
    pGame->pOverText = createText(pGame->pRenderer,238,168,65,pGame->pFont,"Game over",WINDOW_WIDTH/2,WINDOW_HEIGHT/2);
    pGame->pStartText = createText(pGame->pRenderer,238,168,65,pGame->pScoreFont,"Press space to join",WINDOW_WIDTH/2,WINDOW_HEIGHT/2+100);
    pGame->pWaitingText = createText(pGame->pRenderer,238,168,65,pGame->pScoreFont,"Waiting for server...",WINDOW_WIDTH/2,WINDOW_HEIGHT/2+100);
    for(int i=0;i<MAX_ROCKETS;i++){
        if(!pGame->pRocket[i]){
            printf("Error: %s\n",SDL_GetError());
            close(pGame);
            return 0;
        }
    }
    if(!pGame->pOverText || !pGame->pStartText || !pGame->pWaitingText){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }
    pGame->state = START;

    return 1;
}

void run(Game *pGame){
    int close_requested = 0;
    SDL_Event event;
    ClientData cData;
    
    int joining = 0;
    while(!close_requested){
        switch (pGame->state)
        {
            case ONGOING:
                while(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)){
                    updateWithServerData(pGame);
                }			                    

                if(SDL_PollEvent(&event)){
                    if(event.type==SDL_QUIT) close_requested = 1;
                    else handleInput(pGame,&event);
                }
                for(int i=0;i<MAX_ROCKETS;i++)
                    updateRocket(pGame->pRocket[i]);
                SDL_SetRenderDrawColor(pGame->pRenderer,0,0,0,255);
                SDL_RenderClear(pGame->pRenderer);
                SDL_SetRenderDrawColor(pGame->pRenderer,230,230,230,255);
                drawStars(pGame->pStars,pGame->pRenderer);
                for(int i=0;i<MAX_ROCKETS;i++)
                    drawRocket(pGame->pRocket[i]);
                SDL_RenderPresent(pGame->pRenderer);
                
                break;
            case GAME_OVER:
                drawText(pGame->pOverText);
            case START:
                if(!joining){
                    drawText(pGame->pStartText);
                }else{
                    SDL_SetRenderDrawColor(pGame->pRenderer,0,0,0,255);
                    SDL_RenderClear(pGame->pRenderer);
                    SDL_SetRenderDrawColor(pGame->pRenderer,230,230,230,255);
                    drawText(pGame->pWaitingText);
                }
                SDL_RenderPresent(pGame->pRenderer);
                if(SDL_PollEvent(&event)){
                    if(event.type==SDL_QUIT) close_requested = 1;
                    else if(!joining && event.type==SDL_KEYDOWN && event.key.keysym.scancode==SDL_SCANCODE_SPACE){
                        joining = 1;
                        cData.command=READY;
                        cData.playerNumber=-1;
                        memcpy(pGame->pPacket->data, &cData, sizeof(ClientData));
		                pGame->pPacket->len = sizeof(ClientData);
                    }
                }
                if(joining) SDLNet_UDP_Send(pGame->pSocket, -1,pGame->pPacket);
                if(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)){
                    updateWithServerData(pGame);
                    if(pGame->state == ONGOING) joining = 0;
                }                
                break;
        }
        //SDL_Delay(1000/60-15);//might work when you run on different processors
    }
}

void updateWithServerData(Game *pGame){
    ServerData sData;
    memcpy(&sData, pGame->pPacket->data, sizeof(ServerData));
    pGame->rocketNr = sData.playerNr;
    pGame->state = sData.gState;
    for(int i=0;i<MAX_ROCKETS;i++){
        updateRocketWithRecievedData(pGame->pRocket[i],&(sData.rockets[i]));
    }
}

void handleInput(Game *pGame,SDL_Event *pEvent){
    if(pEvent->type == SDL_KEYDOWN){
        ClientData cData;
        cData.playerNumber = pGame->rocketNr;
        switch(pEvent->key.keysym.scancode){
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                accelerate(pGame->pRocket[pGame->rocketNr]);
                cData.command = ACC;
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                turnLeft(pGame->pRocket[pGame->rocketNr]);
                cData.command = LEFT;
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                turnRight(pGame->pRocket[pGame->rocketNr]);
                cData.command = RIGHT;
                break;
            case SDL_SCANCODE_SPACE:
                fireRocket(pGame->pRocket[pGame->rocketNr]);
                cData.command = FIRE;
                break;
        }
        memcpy(pGame->pPacket->data, &cData, sizeof(ClientData));
		pGame->pPacket->len = sizeof(ClientData);
        SDLNet_UDP_Send(pGame->pSocket, -1,pGame->pPacket);
    }
}

void close(Game *pGame){
    for(int i=0;i<MAX_ROCKETS;i++) if(pGame->pRocket[i]) destroyRocket(pGame->pRocket[i]);
    if(pGame->pStars) destroyStars(pGame->pStars);
    if(pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if(pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);

    if(pGame->pWaitingText) destroyText(pGame->pWaitingText);
    if(pGame->pOverText) destroyText(pGame->pOverText);
    if(pGame->pStartText) destroyText(pGame->pStartText);   
    if(pGame->pFont) TTF_CloseFont(pGame->pFont);
    if(pGame->pScoreFont) TTF_CloseFont(pGame->pScoreFont);



    SDLNet_Quit();
    TTF_Quit();    
    SDL_Quit();
}