#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_net.h>
#include <time.h>

#include "bullet.h"
#include "cannon.h"
#include "enemy_1.h"
#include "enemy_2.h"
#include "enemy_3.h"
#include "menu.h"
#include "ship.h"
#include "ship_data.h"
#include "sound.h"
#include "stars.h"
#include "text.h"
#include "tick.h"

#define MUSIC_FILEPATH "../lib/resources/music.wav"
#define TOO_MANY_CLIENTS -1

typedef struct {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ship *pShips[MAX_PLAYERS];
    Cannon *pCannons[MAX_PLAYERS];
    int nrOfShips, nrOfClients;
    GameState state;
    Mix_Music *pMusic;
    TTF_Font *pFont, *pSmallFont;
    Text *pStartText, *pGameName, *pExitText, *pLobbyText, *pTimer, *pOne, *pTwo, *pThree, *pFour,
        *pHardModifiers, *pEasyModifiers, *pEasyMod1, *pEasyMod2, *pEasyMod3, *phardMod1, *phardMod2, *phardMod3,
        *pScoreMod;
    ClientCommand command;
    IPaddress clients[MAX_PLAYERS];
    UDPsocket pSocket;
    UDPpacket *pPacket;
    ServerData serverData;
    bool isRunning, isShooting;
    Cannon *pCannon;
    int nrOfEnemiesToSpawn_1, nrOfEnemies_1, nrOfEnemiesToSpawn_2, nrOfEnemies_2;
    EnemyImage *pEnemy_1Image;
    EnemyImage_2 *pEnemy_2Image;
    Enemy *pEnemies_1[MAX_ENEMIES];
    Enemy_2 *pEnemies_2[MAX_ENEMIES];
    EnemyImage_3 *pEnemy_3Image;
    Enemy_3 *pEnemies_3[1];
    int nrOfEnemies_3, nrOfEnemiesToSpawn_3;
    int map;
    bool showEasyMods, showHardMods;
    int NrOfChosenPlayers;
    int startTime;
    int gameTime;
    int nrOfEasyMods, nrOfHardMods;
    int easyMods[3], hardMods[3];
    Stars *pStars;
    int killedEnemies1, killedEnemies2, killedEnemies3;
    float score;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void handleStartState(Game *pGame);
void handleOngoingState(Game *pGame);
void handleLobbyState(Game *pGame);
void addClient(Game *pGame);
void closeGame(Game *pGame);
int getClientIndex(Game *pGame, IPaddress *clientAddr);
void sendServerData(Game *pGame);
void spawnEnemies_1(Game *pGame, int amount);
void updateEnemies_1(Game *pGame, int *amount);
bool areTheyAllDead_1(Game *pGame);
void printPacketsData(Game *pGame);
void spawnEnemies_2(Game *pGame, int amount);
void updateEnemies_2(Game *pGame, int *amount);
bool areAllEnemy_2_Dead(Game *pGame);
void spawnBoss(Game *pGame);
void resetBoss(Game *pGame);
void updateBoss(Game *pGame);
void printPacketsData(Game *pGame);
bool arePlayersDead(Game *pGame);
void handleGameOverState(Game *pGame);
void resetGameState(Game *pGame);
void updateGameTime(Game *pGame);
int getTime(Game *pGame);
int getScoreMod(Game *pGame);
void updateScoreMod(Game *pGame);
float getSessionScore(Game *pGame);
bool areTheyAllDead_2(Game *pGame);

int main(int argc, char **argv) {
    Game game = {0};
    if (!initiate(&game)) {
        closeGame(&game);
        return 1;
    }
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
        return 0;
    }
    if (TTF_Init() != 0) {
        printf("Error: %s\n", TTF_GetError());
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
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1,
                                          SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        return 0;
    }
    pGame->pFont = TTF_OpenFont("../lib/resources/vermin.ttf", 100);
    pGame->pSmallFont = TTF_OpenFont("../lib/resources/vermin.ttf", 25);
    if (!pGame->pFont) {
        printf("Error: %s\n", TTF_GetError());
        return 0;
    }
    // pGame->pStartText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Start", WINDOW_WIDTH / 3, WINDOW_HEIGHT / 2 + 100);
    pGame->pGameName = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Solar Defence", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4 - 50);
    pGame->pExitText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Exit", WINDOW_WIDTH / 2, WINDOW_HEIGHT - 75);
    pGame->pLobbyText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Waiting on clients...", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    pGame->pStartText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "START", WINDOW_WIDTH / 2, WINDOW_HEIGHT - 270);
    pGame->pHardModifiers = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "+", WINDOW_WIDTH / 2 + 225, WINDOW_HEIGHT - 270);
    pGame->pEasyModifiers = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "-", WINDOW_WIDTH / 2 - 225, WINDOW_HEIGHT - 270);
    pGame->pOne = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "1", 300, WINDOW_HEIGHT - 450);
    pGame->pTwo = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "2", 525, WINDOW_HEIGHT - 450);
    pGame->pThree = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "3", 750, WINDOW_HEIGHT - 450);
    pGame->pFour = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "4", 975, WINDOW_HEIGHT - 450);
    pGame->phardMod1 = createText(pGame->pRenderer, 238, 168, 65, pGame->pSmallFont, "2X ENEMIES", WINDOW_WIDTH - 145, WINDOW_HEIGHT - 550);
    pGame->phardMod2 = createText(pGame->pRenderer, 238, 168, 65, pGame->pSmallFont, "0.5X DAMAGE GIVEN", WINDOW_WIDTH - 145, WINDOW_HEIGHT - 400);
    pGame->phardMod3 = createText(pGame->pRenderer, 238, 168, 65, pGame->pSmallFont, "2X DAMAGE TAKEN", WINDOW_WIDTH - 145, WINDOW_HEIGHT - 250);
    pGame->pEasyMod1 = createText(pGame->pRenderer, 238, 168, 65, pGame->pSmallFont, "0.5X ENEMIES", 155, WINDOW_HEIGHT - 550);
    pGame->pEasyMod2 = createText(pGame->pRenderer, 238, 168, 65, pGame->pSmallFont, "2X DAMAGE GIVEN", 155, WINDOW_HEIGHT - 400);
    pGame->pEasyMod3 = createText(pGame->pRenderer, 238, 168, 65, pGame->pSmallFont, "0.5X DAMAGE TAKEN", 155, WINDOW_HEIGHT - 250);
    pGame->pStars = createStars(WINDOW_WIDTH * WINDOW_HEIGHT / 10000, WINDOW_WIDTH, WINDOW_HEIGHT);

    if (!pGame->pFont) {
        printf("Error: %s\n", TTF_GetError());
        return 0;
    }
    if (!(pGame->pSocket = SDLNet_UDP_Open(SERVER_PORT))) {
        printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }
    if (!(pGame->pPacket = SDLNet_AllocPacket(5120))) {
        printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 0;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        pGame->pShips[i] = createShip(i, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
        pGame->pCannons[i] = createCannon(pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    pGame->nrOfShips = MAX_PLAYERS;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!pGame->pShips[i] || !pGame->pCannons[i]) {
            printf("Error: %s\n", SDL_GetError());
            return 0;
        }
    }
    if (!initMusic(&pGame->pMusic, MUSIC_FILEPATH)) {
        printf("Error: %s\n", Mix_GetError());
        return 0;
    }
    pGame->pEnemy_1Image = initiateEnemy(pGame->pRenderer);
    pGame->pEnemy_2Image = initiateEnemy_2(pGame->pRenderer);
    pGame->pEnemy_3Image = initiateEnemy_3(pGame->pRenderer);
    pGame->map = 1;
    pGame->nrOfEnemies_1 = 0;
    pGame->nrOfEnemies_2 = 0;
    pGame->nrOfEnemies_3 = 0;
    pGame->serverData.win = false;
    pGame->showEasyMods = false;
    pGame->showHardMods = false;
    pGame->isRunning = true;
    pGame->state = START;
    pGame->nrOfEasyMods = 0;
    pGame->nrOfHardMods = 0;
    pGame->score = 0;
    for (int i = 0; i < 3; i++) {
        pGame->easyMods[i] = 0;
        pGame->hardMods[i] = 0;
    }
    return 1;
}

void run(Game *pGame) {
    pGame->nrOfEnemiesToSpawn_1 = WAVE_1_EASY_MAP;
    pGame->nrOfEnemiesToSpawn_2 = WAVE_1_EASY_MAP;
    pGame->nrOfEnemiesToSpawn_3 = NROFBOSSES;

    while (pGame->isRunning) {
        switch (pGame->state) {
        case START:
            handleStartState(pGame);
            break;
        case ONGOING: // Lägg till getTicks()
            handleOngoingState(pGame);
            break;
        case LOBBY:
            handleLobbyState(pGame);
            break;
        case GAME_OVER:
            handleGameOverState(pGame);
            break;
        default:
            pGame->state = START;
            break;
        }
    }
}

void handleStartState(
    Game *pGame) {
    SDL_Event event;
    int x, y;
    const SDL_Rect *startRect = getTextRect(pGame->pStartText);
    const SDL_Rect *exitRect = getTextRect(pGame->pExitText);
    const SDL_Rect *EasierRect = getTextRect(pGame->pEasyModifiers);
    const SDL_Rect *harderRect = getTextRect(pGame->pHardModifiers);
    const SDL_Rect *oneRect = getTextRect(pGame->pOne);
    const SDL_Rect *twoRect = getTextRect(pGame->pTwo);
    const SDL_Rect *threeRect = getTextRect(pGame->pThree);
    const SDL_Rect *fourRect = getTextRect(pGame->pFour);
    const SDL_Rect *easyModRect1 = getTextRect(pGame->pEasyMod1);
    const SDL_Rect *easyModRect2 = getTextRect(pGame->pEasyMod2);
    const SDL_Rect *easyModRect3 = getTextRect(pGame->pEasyMod3);
    const SDL_Rect *hardModRect1 = getTextRect(pGame->phardMod1);
    const SDL_Rect *hardModRect2 = getTextRect(pGame->phardMod2);
    const SDL_Rect *hardModRect3 = getTextRect(pGame->phardMod3);
    pGame->NrOfChosenPlayers = 0;
    pGame->serverData.win = false;

    while (pGame->isRunning && pGame->state == START) {
        SDL_GetMouseState(&x, &y);
        SDL_Point mousePoint = {x, y};
        updateScoreMod(pGame);
        if (SDL_PointInRect(&mousePoint, startRect))
            setTextColor(pGame->pStartText, 255, 100, 100, pGame->pFont, "START");
        else
            setTextColor(pGame->pStartText, 238, 168, 65, pGame->pFont, "START");

        if (SDL_PointInRect(&mousePoint, exitRect))
            setTextColor(pGame->pExitText, 255, 100, 100, pGame->pFont, "EXIT");
        else
            setTextColor(pGame->pExitText, 238, 168, 65, pGame->pFont, "EXIT");

        if (SDL_PointInRect(&mousePoint, harderRect))
            setTextColor(pGame->pHardModifiers, 255, 100, 100, pGame->pFont, "+");
        else
            setTextColor(pGame->pHardModifiers, 238, 168, 65, pGame->pFont, "+");

        if (SDL_PointInRect(&mousePoint, EasierRect))
            setTextColor(pGame->pEasyModifiers, 255, 100, 100, pGame->pFont, "-");
        else
            setTextColor(pGame->pEasyModifiers, 238, 168, 65, pGame->pFont, "-");

        if (SDL_PointInRect(&mousePoint, oneRect))
            setTextColor(pGame->pOne, 255, 100, 100, pGame->pFont, "1");
        else {
            if (pGame->NrOfChosenPlayers == 1)
                setTextColor(pGame->pOne, 0, 200, 0, pGame->pFont, "1");
            else
                setTextColor(pGame->pOne, 238, 168, 65, pGame->pFont, "1");
        }
        if (SDL_PointInRect(&mousePoint, twoRect))
            setTextColor(pGame->pTwo, 255, 100, 100, pGame->pFont, "2");
        else {
            if (pGame->NrOfChosenPlayers == 2)
                setTextColor(pGame->pTwo, 0, 200, 0, pGame->pFont, "2");
            else
                setTextColor(pGame->pTwo, 238, 168, 65, pGame->pFont, "2");
        }

        if (SDL_PointInRect(&mousePoint, threeRect))
            setTextColor(pGame->pThree, 255, 100, 100, pGame->pFont, "3");
        else {
            if (pGame->NrOfChosenPlayers == 3)
                setTextColor(pGame->pThree, 0, 200, 0, pGame->pFont, "3");
            else
                setTextColor(pGame->pThree, 238, 168, 65, pGame->pFont, "3");
        }
        if (SDL_PointInRect(&mousePoint, fourRect))
            setTextColor(pGame->pFour, 255, 100, 100, pGame->pFont, "4");
        else {
            if (pGame->NrOfChosenPlayers == 4)
                setTextColor(pGame->pFour, 0, 200, 0, pGame->pFont, "4");
            else
                setTextColor(pGame->pFour, 238, 168, 65, pGame->pFont, "4");
        }
        if (SDL_PointInRect(&mousePoint, easyModRect1)) {
            setTextColor(pGame->pEasyMod1, 255, 100, 100, pGame->pSmallFont, "0.5X ENEMIES");
        } else {
            if (pGame->easyMods[0] == 1) {
                setTextColor(pGame->pEasyMod1, 0, 200, 0, pGame->pSmallFont, "0.5X ENEMIES");
            } else {
                setTextColor(pGame->pEasyMod1, 238, 168, 65, pGame->pSmallFont, "0.5X ENEMIES");
            }
        }
        if (SDL_PointInRect(&mousePoint, easyModRect2)) {
            setTextColor(pGame->pEasyMod2, 255, 100, 100, pGame->pSmallFont, "2X DAMAGE GIVEN");
        } else {
            if (pGame->easyMods[1] == 1) {
                setTextColor(pGame->pEasyMod2, 0, 200, 0, pGame->pSmallFont, "2X DAMAGE GIVEN");
            } else {
                setTextColor(pGame->pEasyMod2, 238, 168, 65, pGame->pSmallFont, "2X DAMAGE GIVEN");
            }
        }
        if (SDL_PointInRect(&mousePoint, easyModRect3)) {
            setTextColor(pGame->pEasyMod3, 255, 100, 100, pGame->pSmallFont, "0.5X DAMAGE TAKEN");
        } else {
            if (pGame->easyMods[2] == 1) {
                setTextColor(pGame->pEasyMod3, 0, 200, 0, pGame->pSmallFont, "0.5X DAMAGE TAKEN");
            } else {
                setTextColor(pGame->pEasyMod3, 238, 168, 65, pGame->pSmallFont, "0.5X DAMAGE TAKEN");
            }
        }
        if (SDL_PointInRect(&mousePoint, hardModRect1)) {
            setTextColor(pGame->phardMod1, 255, 100, 100, pGame->pSmallFont, "2X ENEMIES");
        } else {
            if (pGame->hardMods[0] == 1) {
                setTextColor(pGame->phardMod1, 0, 200, 0, pGame->pSmallFont, "2X ENEMIES");
            } else {
                setTextColor(pGame->phardMod1, 238, 168, 65, pGame->pSmallFont, "2X ENEMIES");
            }
        }
        if (SDL_PointInRect(&mousePoint, hardModRect2)) {
            setTextColor(pGame->phardMod2, 255, 100, 100, pGame->pSmallFont, "0.5X DAMAGE GIVEN");
        } else {
            if (pGame->hardMods[1] == 1) {
                setTextColor(pGame->phardMod2, 0, 200, 0, pGame->pSmallFont, "0.5X DAMAGE GIVEN");
            } else {
                setTextColor(pGame->phardMod2, 238, 168, 65, pGame->pSmallFont, "0.5X DAMAGE GIVEN");
            }
        }
        if (SDL_PointInRect(&mousePoint, hardModRect3)) {
            setTextColor(pGame->phardMod3, 255, 100, 100, pGame->pSmallFont, "2X DAMAGE TAKEN");
        } else {
            if (pGame->hardMods[2] == 1)
                setTextColor(pGame->phardMod3, 0, 200, 0, pGame->pSmallFont, "2X DAMAGE TAKEN");
            else
                setTextColor(pGame->phardMod3, 238, 168, 65, pGame->pSmallFont, "2X DAMAGE TAKEN");
        }
        SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pGame->pRenderer);
        SDL_SetRenderDrawColor(pGame->pRenderer, 255, 255, 255, 255);
        drawStars(pGame->pStars, pGame->pRenderer);
        drawModifierMenu(pGame->pRenderer, pGame->pFont);
        drawSoreModifier(pGame->pRenderer);
        if (pGame->pScoreMod) drawText(pGame->pScoreMod);
        drawText(pGame->pGameName);
        drawText(pGame->pStartText);
        drawText(pGame->pExitText);
        drawText(pGame->pEasyModifiers);
        drawText(pGame->pHardModifiers);
        drawText(pGame->pOne);
        drawText(pGame->pTwo);
        drawText(pGame->pThree);
        drawText(pGame->pFour);
        if (pGame->showEasyMods == true) {
            DrawModifiersToMakeGameEasier(pGame->pRenderer, pGame->pFont);
            drawText(pGame->pEasyMod1);
            drawText(pGame->pEasyMod2);
            drawText(pGame->pEasyMod3);
        }
        if (pGame->showHardMods == true) {
            DrawModifiersToMakeGameHarder(pGame->pRenderer, pGame->pFont);
            drawText(pGame->phardMod1);
            drawText(pGame->phardMod2);
            drawText(pGame->phardMod3);
        }
        SDL_RenderPresent(pGame->pRenderer);

        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pGame->isRunning = false;
            } else if (SDL_PointInRect(&mousePoint, startRect) &&
                       event.type == SDL_MOUSEBUTTONDOWN && pGame->NrOfChosenPlayers > 0) {
                pGame->state = LOBBY;
            } else if (SDL_PointInRect(&mousePoint, exitRect) &&
                       event.type == SDL_MOUSEBUTTONDOWN) {
                pGame->isRunning = false;
            } else if (SDL_PointInRect(&mousePoint, oneRect) &&
                       event.type == SDL_MOUSEBUTTONDOWN) {
                pGame->NrOfChosenPlayers = 1;
            } else if (SDL_PointInRect(&mousePoint, twoRect) &&
                       event.type == SDL_MOUSEBUTTONDOWN) {
                pGame->NrOfChosenPlayers = 2;
            } else if (SDL_PointInRect(&mousePoint, threeRect) &&
                       event.type == SDL_MOUSEBUTTONDOWN) {
                pGame->NrOfChosenPlayers = 3;
            } else if (SDL_PointInRect(&mousePoint, fourRect) &&
                       event.type == SDL_MOUSEBUTTONDOWN) {
                pGame->NrOfChosenPlayers = 4;
            } else if (SDL_PointInRect(&mousePoint, EasierRect) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showEasyMods == false) {
                pGame->showEasyMods = true;
            } else if (SDL_PointInRect(&mousePoint, EasierRect) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showEasyMods == true) {
                pGame->showEasyMods = false;
            } else if (SDL_PointInRect(&mousePoint, harderRect) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showHardMods == false) {
                pGame->showHardMods = true;
            } else if (SDL_PointInRect(&mousePoint, harderRect) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showHardMods == true) {
                pGame->showHardMods = false;
            } else if (SDL_PointInRect(&mousePoint, easyModRect1) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showEasyMods == true && pGame->easyMods[0] != 1) {
                pGame->easyMods[0] = 1;
                pGame->nrOfEasyMods++;
            } else if (SDL_PointInRect(&mousePoint, easyModRect1) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showEasyMods == true && pGame->easyMods[0] == 1) {
                pGame->easyMods[0] = 0;
                pGame->nrOfEasyMods--;
            } else if (SDL_PointInRect(&mousePoint, easyModRect2) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showEasyMods == true && pGame->easyMods[1] != 1) {
                pGame->easyMods[1] = 1;
                pGame->nrOfEasyMods++;
            } else if (SDL_PointInRect(&mousePoint, easyModRect2) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showEasyMods == true && pGame->easyMods[1] == 1) {
                pGame->easyMods[1] = 0;
                pGame->nrOfEasyMods--;
            } else if (SDL_PointInRect(&mousePoint, easyModRect3) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showEasyMods == true && pGame->easyMods[2] != 1) {
                pGame->easyMods[2] = 1;
                pGame->nrOfEasyMods++;
            } else if (SDL_PointInRect(&mousePoint, easyModRect3) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showEasyMods == true && pGame->easyMods[2] == 1) {
                pGame->easyMods[2] = 0;
                pGame->nrOfEasyMods--;
            } else if (SDL_PointInRect(&mousePoint, hardModRect1) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showHardMods == true && pGame->hardMods[0] != 1) {
                pGame->hardMods[0] = 1;
                pGame->nrOfHardMods++;
            } else if (SDL_PointInRect(&mousePoint, hardModRect1) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showHardMods == true && pGame->hardMods[0] == 1) {
                pGame->hardMods[0] = 0;
                pGame->nrOfHardMods--;
            } else if (SDL_PointInRect(&mousePoint, hardModRect2) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showHardMods == true && pGame->hardMods[1] != 1) {
                pGame->hardMods[1] = 1;
                pGame->nrOfHardMods++;
            } else if (SDL_PointInRect(&mousePoint, hardModRect2) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showHardMods == true && pGame->hardMods[1] == 1) {
                pGame->hardMods[1] = 0;
                pGame->nrOfHardMods--;
            } else if (SDL_PointInRect(&mousePoint, hardModRect3) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showHardMods == true && pGame->hardMods[2] != 1) {
                pGame->hardMods[2] = 1;
                pGame->nrOfHardMods++;
            } else if (SDL_PointInRect(&mousePoint, hardModRect3) && event.type == SDL_MOUSEBUTTONDOWN && pGame->showHardMods == true && pGame->hardMods[2] == 1) {
                pGame->hardMods[2] = 0;
                pGame->nrOfHardMods--;
            }
        }
        SDL_Delay(8);
    }
}

void handleOngoingState(Game *pGame) {
    SDL_Event event;
    ClientData cData;
    Uint32 now = 0, delta = 0, lastUpdate = SDL_GetTicks();
    const Uint32 tickInterval = 16;
    pGame->nrOfEnemies_1 = 0;
    pGame->nrOfEnemies_2 = 0;
    pGame->killedEnemies1 = 0;
    pGame->killedEnemies2 = 0;
    pGame->killedEnemies3 = 0;
    SDL_Rect emptyRect = {0, 0, 0, 0}, rectArray[MAX_PROJECTILES] = {0, 0, 0, 0};
    pGame->map = 1;
    pGame->score = 0;
    pGame->startTime = SDL_GetTicks64();
    pGame->gameTime = -2;
    bool upgradesDone = 0;
    int dmgGiven = REGULAR_DMG_GIVEN;
    int dmgTaken = REGULAR_DMG_TAKEN;
    int count = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        pGame->serverData.clientStatus[i] = 0;
    }
    for (int i = 0; i < DATA_STORED; i++) {
        for (int j = 0; j < MAX_PLAYERS; j++) {
            pGame->serverData.saveData[i][j] = 0;
        }
    }

    if (pGame->easyMods[1] == 1) dmgGiven *= 2;
    if (pGame->hardMods[1] == 1) dmgGiven /= 2;
    if (pGame->easyMods[2] == 1) dmgTaken /= 2;
    if (pGame->hardMods[2] == 1) dmgTaken *= 2;
    if (pGame->nrOfEnemies_3 == 0) {
        spawnBoss(pGame);
    }
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
            memcpy(&cData, pGame->pPacket->data, sizeof(ClientData));
            clientIndex = getClientIndex(pGame, &pGame->pPacket->address);
            pGame->map = cData.map;

            if (clientIndex >= 0 && clientIndex < pGame->NrOfChosenPlayers) {
                applyShipCommand(pGame->pShips[clientIndex], cData.command);
                if (cData.isShooting) {
                    setShoot(pGame->pShips[clientIndex], true);
                }
                for (int i = 0; i < DATA_STORED; i++) {
                    pGame->serverData.saveData[i][clientIndex] = cData.saveData[i];
                }
            }
        }
        for (int i = 0; i < pGame->NrOfChosenPlayers; i++) {
            if (pGame->serverData.saveData[2][i] == 1) {
                ShipSpeedUpgrade(pGame->pShips[i]);
            }
            if (pGame->serverData.saveData[3][i] == 1) // yes this was the easiest solution i could think of.
            {
                // the player who has this upgrade got scammed
            }
            if (pGame->serverData.saveData[4][i] == 1) {
                shipHealthUpgrade(pGame->pShips[i]);
                cannonHpUpgrade(pGame->pCannons[i]);
            }
        }
        if (timeToUpdate(&lastUpdate, tickInterval)) {
            for (int i = 0; i < pGame->NrOfChosenPlayers; i++) {
                if (pGame->pShips[i]) {
                    if (isCannonShooting(pGame->pShips[i]) && isPlayerDead(pGame->pShips[i]) == false) {
                        handleCannonEvent(pGame->pCannons[i]);
                    }
                    update_projectiles(delta);
                    updateShipVelocity(pGame->pShips[i]);
                    updateShipOnServer(pGame->pShips[i]);
                    updateCannon(pGame->pCannons[i], pGame->pShips[i]);
                }
            }
            updateGameTime(pGame);
            if (pGame->gameTime == 30) {
                SDL_Delay(3000);
            }
            if (pGame->map == 2) {
                for (int i = 0; i < pGame->nrOfEnemies_1 && i < MAX_ENEMIES; i++) {
                    damageEnemy(pGame->pEnemies_1[i], 100, i);
                }
            }

            if (pGame->map == 0)
                updateEnemies_1(pGame, &pGame->nrOfEnemiesToSpawn_1);
            else if (pGame->map == 2) {
                updateEnemies_2(pGame, &pGame->nrOfEnemiesToSpawn_2);
                updateEnemy_3(pGame->pEnemies_3[0]);
            }
            for (int i = 0; i < pGame->nrOfEnemies_1 && i < MAX_ENEMIES; i++) {
                updateEnemy(pGame->pEnemies_1[i]);
            }
            for (int i = 0; i < pGame->nrOfEnemies_2 && i < MAX_ENEMIES; i++) {
                updateEnemy_2(pGame->pEnemies_2[i]);
            }
            SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
            SDL_RenderClear(pGame->pRenderer);
            if (pGame->pTimer) drawText(pGame->pTimer);
            for (int i = 0; i < pGame->NrOfChosenPlayers; i++) {
                render_projectiles(pGame->pRenderer);
                drawShip(pGame->pShips[i]);
                drawCannon(pGame->pCannons[i]);
            }
            for (int i = 0; i < pGame->nrOfEnemies_1 && i < MAX_ENEMIES; i++) {
                if (isEnemyActive(pGame->pEnemies_1[i]) == true) {
                    drawEnemy(pGame->pEnemies_1[i]);
                }
            }
            for (int i = 0; i < pGame->nrOfEnemies_2 && i < MAX_ENEMIES; i++) {
                if (isEnemy_2Active(pGame->pEnemies_2[i])) drawEnemy_2(pGame->pEnemies_2[i]);
            }
            if (isEnemy_3Active(pGame->pEnemies_3[0]) == true) {
                drawEnemy_3(pGame->pEnemies_3[0]);
            }
            for (int i = 0; i < pGame->NrOfChosenPlayers; i++) {
                for (int j = 0; j < pGame->nrOfEnemies_1 && j < MAX_ENEMIES; j++) {
                    if (shipCollision(pGame->pShips[i], getRectEnemy(pGame->pEnemies_1[j]))) {
                        damageEnemy(pGame->pEnemies_1[j], 100, j);
                        damageShip(pGame->pShips[i], dmgTaken);
                        damageCannon(pGame->pCannons[i], dmgTaken);
                        if (isPlayerDead(pGame->pShips[i])) {
                            printf("Player %d killed by enemy1\n", i);
                        }
                    }
                }
                for (int j = 0; j < pGame->nrOfEnemies_2 && j < MAX_ENEMIES; j++) {
                    if (shipCollision(pGame->pShips[i], getRectEnemy_2(pGame->pEnemies_2[j]))) {
                        damageEnemy_2(pGame->pEnemies_2[j], 100, j);
                        damageShip(pGame->pShips[i], dmgTaken);
                        damageCannon(pGame->pCannons[i], dmgTaken);
                        if (isPlayerDead(pGame->pShips[i])) {
                            printf("Player %d killed by enemy2\n", i);
                        }
                    }
                }
                for (int j = 0; j < pGame->nrOfEnemies_3 && j < MAX_ENEMIES; j++) {
                    if (shipCollision(pGame->pShips[i], getRectEnemy_3(pGame->pEnemies_3[j]))) {
                        damageEnemy_3(pGame->pEnemies_3[j], 200, j);
                        damageShip(pGame->pShips[i], 100);
                        damageCannon(pGame->pCannons[i], 100);
                        if (isPlayerDead(pGame->pShips[i])) {
                            printf("Player %d killed by enemy3\n", i);
                        }
                    }
                }
            }
            getProjectileRects(rectArray);
            for (int i = 0; i < MAX_PROJECTILES; i++) {
                SDL_Rect bulletRect = rectArray[i];
                for (int k = 0; k < pGame->nrOfEnemies_1; k++) {
                    SDL_Rect enemyRect = getRectEnemy(pGame->pEnemies_1[k]);
                    if (SDL_HasIntersection(&enemyRect, &bulletRect)) {
                        damageEnemy(pGame->pEnemies_1[k], dmgGiven, k);
                        if (isEnemyActive(pGame->pEnemies_1[k]) == false) {
                            (pGame->killedEnemies1)++;
                        }
                        removeProjectile(i);
                        rectArray[i] = emptyRect;
                        for (int j = 0; j < pGame->NrOfChosenPlayers; j++) {
                            setBulletToRemove(pGame->pShips[j], i);
                        }
                    }
                }
                for (int k = 0; k < pGame->nrOfEnemies_2; k++) {
                    SDL_Rect enemyRect2 = getRectEnemy_2(pGame->pEnemies_2[k]);
                    if (SDL_HasIntersection(&enemyRect2, &bulletRect)) {
                        damageEnemy_2(pGame->pEnemies_2[k], dmgGiven, k);
                        if (isEnemy_2Active(pGame->pEnemies_2[k]) == false) {
                            (pGame->killedEnemies2)++;
                        }
                        removeProjectile(i);
                        rectArray[i] = emptyRect;
                        for (int j = 0; j < pGame->NrOfChosenPlayers; j++) {
                            setBulletToRemove(pGame->pShips[j], i);
                        }
                    }
                }
                for (int k = 0; k < pGame->nrOfEnemies_3; k++) {
                    SDL_Rect enemyRect3 = getRectEnemy_3(pGame->pEnemies_3[k]);
                    if (SDL_HasIntersection(&enemyRect3, &bulletRect)) {
                        damageEnemy_3(pGame->pEnemies_3[k], dmgGiven, k);
                        if (isEnemy_3Active(pGame->pEnemies_3[k]) == false) {
                            (pGame->killedEnemies3)++;
                            pGame->score = getSessionScore(pGame);
                            pGame->serverData.sessionScore = pGame->score;
                            pGame->state = GAME_OVER;
                            pGame->serverData.gState = pGame->state;
                            pGame->serverData.win = true;
                        }
                        removeProjectile(i);
                        rectArray[i] = emptyRect;
                        for (int j = 0; j < pGame->NrOfChosenPlayers; j++)
                            setBulletToRemove(pGame->pShips[j], i);
                    }
                }
            }
            SDL_RenderPresent(pGame->pRenderer);
            sendServerData(pGame);
            for (int i = 0; i < MAX_PLAYERS; i++) {
                setShoot(pGame->pShips[i], false);
                setBulletToRemove(pGame->pShips[i], -1);
            }
        }
    }
}

void handleLobbyState(Game *pGame) {
    SDL_Event event;
    printf("Server is listening on port %hu...\n", SERVER_PORT);
    while (pGame->isRunning && pGame->state == LOBBY) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pGame->isRunning = false;
                return;
            }
        }
        if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket) == 1) {
            addClient(pGame);
            printf("After addClient()");
            if (pGame->nrOfClients == pGame->NrOfChosenPlayers) {
                pGame->state = ONGOING;
                for (int i = 0; i < pGame->NrOfChosenPlayers; i++) {
                    const char *msg = "ONGOING";
                    memcpy(pGame->pPacket->data, msg, strlen(msg) + 1);
                    pGame->pPacket->address = pGame->clients[i];
                    SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
                }
            }
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
    if (strncmp((char *)pGame->pPacket->data, "TRYING TO CONNECT", 17) == 0) {
        printf("Received connection request.\n");
        int clientIndex = getClientIndex(pGame, &pGame->pPacket->address);
        if (clientIndex >= 0 && clientIndex < pGame->NrOfChosenPlayers) {
            pGame->serverData.sDPlayerId = clientIndex;
            memcpy(pGame->pPacket->data, &pGame->serverData, sizeof(ServerData));
            pGame->pPacket->len = sizeof(ServerData);
            pGame->pPacket->address = pGame->clients[clientIndex];
            if (SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket)) {
                printf("Sent connection confirmation to client %d. \n", clientIndex);
                printf("*Sending first packet to client %d:\n", clientIndex);
                printPacketsData(pGame);
            } else
                printf("Failed to Send connection confirmation to client %d.\n", clientIndex);
        }
    }
}

void handleGameOverState(Game *pGame) {
    Text *pGameOverText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont,
                                     "GAME OVER", WINDOW_WIDTH / 2, 150);
    Text *pRestartServer = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont,
                                      "RESTART SERVER", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    SDL_RenderPresent(pGame->pRenderer);

    const SDL_Rect *pRestartRect =
        getTextRect(pRestartServer);
    SDL_Event event;
    printf("Score: %.2f \n", pGame->serverData.sessionScore);
    while (pGame->isRunning) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        SDL_Point mousePoint = {x, y};
        if (SDL_PointInRect(&mousePoint, pRestartRect))
            setTextColor(pRestartServer, 255, 100, 100, pGame->pFont, "RETURN TO MAIN MENU");
        else
            setTextColor(pRestartServer, 238, 168, 65, pGame->pFont, "RETURN TO MAIN MENU");

        SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pGame->pRenderer);
        drawText(pGameOverText);
        drawText(pRestartServer);
        SDL_RenderPresent(pGame->pRenderer);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pGame->isRunning = false;
            } else if (SDL_PointInRect(&mousePoint, pRestartRect) && event.type == SDL_MOUSEBUTTONDOWN) {
                resetGameState(pGame);
                pGame->state = START;
                pGame->serverData.gState = pGame->state;
                run(pGame);
            }
        }
    }
}

void sendServerData(Game *pGame) {
    if (pGame->nrOfClients == 0) return; // No clients connected
    pGame->serverData.gState = pGame->state;
    pGame->serverData.nrOfPlayers = pGame->NrOfChosenPlayers;
    for (int i = 0; i < pGame->NrOfChosenPlayers; i++)
        getShipDataPackage(pGame->pShips[i], &pGame->serverData.ships[i]);

    for (int i = 0; i < pGame->nrOfEnemies_1 && i < MAX_ENEMIES; i++)
        getEnemy_1_DataPackage(pGame->pEnemies_1[i], &pGame->serverData.enemies_1[i]);
    pGame->serverData.nrOfEnemies_1 = pGame->nrOfEnemies_1;
    for (int i = 0; i < pGame->nrOfEnemies_2 && i < MAX_ENEMIES; i++)
        getEnemy_2_DataPackage(pGame->pEnemies_2[i], &pGame->serverData.enemies_2[i]);
    pGame->serverData.nrOfEnemies_2 = pGame->nrOfEnemies_2;
    for (int i = 0; i < pGame->nrOfEnemies_3 && i < NROFBOSSES; i++)
        getEnemy_3_DataPackage(pGame->pEnemies_3[i], &pGame->serverData.enemies_3[i]);
    pGame->serverData.nrOfEnemies_3 = pGame->nrOfEnemies_3;
    for (int i = 0; i < pGame->NrOfChosenPlayers; i++) {
        if (isPlayerDead(pGame->pShips[i]) == true) {
            pGame->serverData.clientStatus[i] = 1;
        }
    }
    if (arePlayersDead(pGame) == true) {
        pGame->score = getSessionScore(pGame);
        pGame->serverData.sessionScore = pGame->score;
        pGame->state = GAME_OVER;
        pGame->serverData.gState = pGame->state;
    }
    for (int i = 0; i < pGame->NrOfChosenPlayers; i++) {
        pGame->serverData.sDPlayerId = i;
        memcpy(pGame->pPacket->data, &(pGame->serverData), sizeof(ServerData));
        pGame->pPacket->len = sizeof(ServerData);
        pGame->pPacket->address = pGame->clients[i];
        SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
    }
}

// Call this func right after sending a packet for debugging!!! ADD MORE STUFF
// WHEN ADDING THEM TO THE SERVERDATA STRUCT!
void printPacketsData(Game *pGame) {
    ServerData *sd = &pGame->serverData;
    printf("Sending a packet with the size of %d bytes \n", pGame->pPacket->len);
    printf("*Sending updated packet to client %d:\n", sd->sDPlayerId);
    printf("gState: %d\n", sd->gState);
    printf("  sDPlayerId (assigned ID): %d\n", sd->sDPlayerId);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        printf("  Ship %d: x = %.2f, y = %.2f, angle = %.2f, alive = %d\n", i, sd->ships[i].x,
               sd->ships[i].y, sd->ships[i].vx, sd->ships[i].vy);
    }
    printf("  Enemies_1: %d active, %d to spawn\n", sd->nrOfEnemies_1, sd->nrOfEnemiesToSpawn_1);
    // Print each enemy_1's data
    for (int i = 0; i < sd->nrOfEnemies_1; i++) {
        printf("    Enemy_1[%d]: x = %.2f, y = %.2f, alive = %d\n", i, sd->enemies_1[i].x,
               sd->enemies_1[i].y, sd->enemies_1[i].active);
    }
    /*printf("  Enemies_2: %d active, %d to spawn\n", sd->nrOfEnemies_2,
    sd->nrOfEnemiesToSpawn_2); Print each enemy_2's data for (int i = 0; i <
    sd->nrOfEnemies_2; i++) { printf("    Enemy_1[%d]: x = %.2f, y = %.2f, alive =
    %d\n", i, sd->enemies_1[i].x, sd->enemies_2[i].y, sd->enemies_2[i].active);
    }*/
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
            printf("Client %d: %d.%d.%d.%d:%d\n", i, (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF, port);
        }
        return pGame->nrOfClients++;
    }
    return -1;
}

void closeGame(Game *pGame) {
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (pGame->pShips[i]) destroyShip(pGame->pShips[i]);
    for (int i = 0; i < MAX_PLAYERS; i++)                          //
        if (pGame->pCannons[i]) destroyCannon(pGame->pCannons[i]); //
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);

    if (pGame->pStartText) destroyText(pGame->pStartText);
    if (pGame->pGameName) destroyText(pGame->pGameName);
    if (pGame->pExitText) destroyText(pGame->pExitText);
    if (pGame->pLobbyText) destroyText(pGame->pLobbyText);
    if (pGame->pTimer) destroyText(pGame->pTimer);
    if (pGame->pOne) destroyText(pGame->pOne);
    if (pGame->pTwo) destroyText(pGame->pTwo);
    if (pGame->pThree) destroyText(pGame->pThree);
    if (pGame->pFour) destroyText(pGame->pFour);
    if (pGame->pHardModifiers) destroyText(pGame->pHardModifiers);
    if (pGame->pEasyModifiers) destroyText(pGame->pEasyModifiers);
    if (pGame->pEasyMod1) destroyText(pGame->pEasyMod1);
    if (pGame->pEasyMod2) destroyText(pGame->pEasyMod2);
    if (pGame->pEasyMod3) destroyText(pGame->pEasyMod3);
    if (pGame->phardMod1) destroyText(pGame->phardMod1);
    if (pGame->phardMod2) destroyText(pGame->phardMod2);
    if (pGame->phardMod3) destroyText(pGame->phardMod3);
    if (pGame->pScoreMod) destroyText(pGame->phardMod3);

    if (pGame->pFont) TTF_CloseFont(pGame->pFont);
    if (pGame->pSmallFont) TTF_CloseFont(pGame->pSmallFont);

    if (pGame->pMusic) closeMusic(pGame->pMusic);
    if (pGame->pSocket) SDLNet_UDP_Close(pGame->pSocket);
    if (pGame->pPacket) SDLNet_FreePacket(pGame->pPacket);

    for (int i = 0; i < MAX_ENEMIES; i++)
        if (pGame->pEnemies_1[i]) destroyEnemy_1(pGame->pEnemies_1[i]);
    if (pGame->pEnemy_1Image) destroyEnemy_1Image(pGame->pEnemy_1Image);

    for (int i = 0; i < MAX_ENEMIES; i++)
        if (pGame->pEnemies_2[i]) destroyEnemy_2(pGame->pEnemies_2[i]);
    if (pGame->pEnemy_2Image) destroyEnemyImage_2(pGame->pEnemy_2Image);

    for (int i = 0; i < NROFBOSSES; i++)
        if (pGame->pEnemies_3[i]) destroyEnemy_3(pGame->pEnemies_3[i]);
    if (pGame->pEnemy_3Image) destroyEnemyImage_3(pGame->pEnemy_3Image);

    SDLNet_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void spawnEnemies_1(Game *pGame, int amount) {
    for (int spawnCount = 0; spawnCount < amount; spawnCount++) {
        pGame->pEnemies_1[pGame->nrOfEnemies_1] =
            createEnemy(pGame->pEnemy_1Image, WINDOW_WIDTH, WINDOW_HEIGHT);
        pGame->nrOfEnemies_1++;
    }
}

void updateEnemies_1(Game *pGame, int *amount) {
    if (areTheyAllDead_1(pGame) == true) {
        (*amount) += 4;
        if (pGame->easyMods[0] == 1) (*amount) /= 2;
        if (pGame->hardMods[0] == 1) (*amount) *= 2;
        if ((*amount) > MAX_ENEMIES) (*amount) = MAX_ENEMIES;
        pGame->nrOfEnemies_1 = 0;
        spawnEnemies_1(pGame, *amount);
    }
}

bool areTheyAllDead_1(Game *pGame) {
    for (int i = 0; i < pGame->nrOfEnemies_1 && i < MAX_ENEMIES; i++) {
        if (isEnemyActive(pGame->pEnemies_1[i]) == true) {
            return false;
        }
    }
    return true;
}

void spawnEnemies_2(Game *pGame, int amount) {
    for (int spawnCount = 0; spawnCount < amount; spawnCount++) {
        pGame->pEnemies_2[pGame->nrOfEnemies_2] =
            createEnemy_2(pGame->pEnemy_2Image, WINDOW_WIDTH, WINDOW_HEIGHT);
        pGame->nrOfEnemies_2++;
    }
}

void updateEnemies_2(Game *pGame, int *amount) {
    if (areTheyAllDead_2(pGame) == true) {
        (*amount) += 4;
        if (pGame->easyMods[0] == 1) (*amount) /= 2;
        if (pGame->hardMods[0] == 1) (*amount) *= 2;
        if ((*amount) > MAX_ENEMIES) (*amount) = MAX_ENEMIES;
        pGame->nrOfEnemies_2 = 0;
        spawnEnemies_2(pGame, *amount);
    }
}

bool areTheyAllDead_2(Game *pGame) {
    for (int i = 0; i < pGame->nrOfEnemies_2 && i < MAX_ENEMIES; i++) {
        if (isEnemy_2Active(pGame->pEnemies_2[i]) == true) {
            return false;
        }
    }
    return true;
}

void spawnBoss(Game *pGame) {
    pGame->pEnemies_3[0] = createEnemy_3(pGame->pEnemy_3Image, WINDOW_WIDTH, WINDOW_HEIGHT);
    pGame->nrOfEnemies_3++;
}

void updateBoss(Game *pGame) {
    if (isEnemy_3Active(pGame->pEnemies_3[0])) {
        updateEnemy_3(pGame->pEnemies_3[0]);
    }
}

bool arePlayersDead(Game *pGame) {
    for (int i = 0; i < pGame->nrOfClients; i++) {
        if (isPlayerDead(pGame->pShips[i]) == false) {
            return false;
        }
    }
    printf("ALL PLAYERS DEAD! \n");
    return true;
}

void resetGameState(Game *pGame) {
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (pGame->pShips[i]) destroyShip(pGame->pShips[i]);
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (pGame->pCannons[i]) destroyCannon(pGame->pCannons[i]);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (pGame->pShips[i]) {
            pGame->pShips[i] = createShip(i, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
            applyShipCommand(pGame->pShips[i], STOP_SHIP);
            resetHealth(pGame->pShips[i]);
        }
        if (pGame->pCannons[i]) {
            pGame->pCannons[i] = createCannon(pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
            resetCannon(pGame->pCannons[i]);
            resetCannonHealth(pGame->pCannons[i]);
        }
        if (!pGame->pShips[i] || !pGame->pCannons[i]) {
            printf("Error: %s\n", SDL_GetError());
            return;
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (pGame->pEnemies_1[i]) {
            destroyEnemy_1(pGame->pEnemies_1[i]);
            pGame->pEnemies_1[i] = NULL;
        }
        if (pGame->pEnemies_2[i]) {
            destroyEnemy_2(pGame->pEnemies_2[i]);
            pGame->pEnemies_2[i] = NULL;
        }
    }
    for (int i = 0; i < NROFBOSSES; i++) {
        if (pGame->pEnemies_3[i]) {
            destroyEnemy_3(pGame->pEnemies_3[i]);
            pGame->pEnemies_3[i] = NULL;
        }
    }
    pGame->nrOfEnemies_1 = 0;
    pGame->nrOfEnemiesToSpawn_1 = WAVE_1_EASY_MAP;
    pGame->nrOfEnemies_2 = 0;
    pGame->nrOfEnemiesToSpawn_2 = WAVE_1_EASY_MAP;
    pGame->nrOfEnemies_3 = 0;
    pGame->nrOfEnemiesToSpawn_3 = NROFBOSSES;
    pGame->nrOfClients = 0;
    pGame->NrOfChosenPlayers = 0;
    pGame->killedEnemies1 = 0;
    pGame->killedEnemies2 = 0;
    pGame->killedEnemies3 = 0;
    pGame->map = 1;
    memset(pGame->clients, 0, sizeof(pGame->clients));
    resetAllBullets();
}

int getTime(Game *pGame) {
    return (SDL_GetTicks64() - pGame->startTime) / 1000;
}

void updateGameTime(Game *pGame) {
    if (getTime(pGame) > pGame->gameTime && pGame->state == ONGOING) {
        (pGame->gameTime)++;
        if (pGame->pTimer) destroyText(pGame->pTimer);
        static char timerString[30];
        sprintf(timerString, "%d", getTime(pGame));
        if (pGame->pFont) {
            pGame->pTimer = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, timerString, WINDOW_WIDTH / 2, 50);
        }
    }
}

void updateScoreMod(Game *pGame) {
    if (pGame->pScoreMod) destroyText(pGame->pScoreMod);
    static char modString[30];
    sprintf(modString, "%d%%", getScoreMod(pGame));
    if (pGame->pFont) {
        if (getScoreMod(pGame) < 100) {
            pGame->pScoreMod = createText(pGame->pRenderer, 175, 0, 0, pGame->pFont, modString, 138, WINDOW_HEIGHT - 60);
        } else if (getScoreMod(pGame) >= 100 && getScoreMod(pGame) <= 150) {
            pGame->pScoreMod = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, modString, 138, WINDOW_HEIGHT - 60);
        } else if (getScoreMod(pGame) > 150 && getScoreMod(pGame) <= 300) {
            pGame->pScoreMod = createText(pGame->pRenderer, 0, 180, 0, pGame->pFont, modString, 138, WINDOW_HEIGHT - 60);
        }
    }
}

int getScoreMod(Game *pGame) {
    int scoreMod = 0;
    if (pGame->NrOfChosenPlayers == 0) {
        return 0;
    }

    else if (pGame->NrOfChosenPlayers == 1) {
        scoreMod = 100;
    } else if (pGame->NrOfChosenPlayers == 2) {
        scoreMod = 133;
    } else if (pGame->NrOfChosenPlayers == 3) {
        scoreMod = 166;
    } else if (pGame->NrOfChosenPlayers == 4) {
        scoreMod = 200;
    }

    for (int i = 0; i < pGame->nrOfEasyMods; i++) {
        scoreMod -= 25;
    }
    for (int i = 0; i < pGame->nrOfHardMods; i++) {
        scoreMod += 25;
    }
    return scoreMod;
}

float getSessionScore(Game *pGame) {
    float score = 0;
    float scoreMod = getScoreMod(pGame) / 100.0f;
    for (int i = 0; i < pGame->killedEnemies1; i++)
        score += 250;
    for (int i = 0; i < pGame->killedEnemies2; i++)
        score += 500;
    for (int i = 0; i < pGame->killedEnemies3; i++)
        score += 10000;
    score *= scoreMod;
    return score;
}