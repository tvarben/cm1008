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
#define ASTEROIDPATH "../lib/resources/Asteroid.png"
#define EARTHPATH "../lib/resources/Earth.png"
#define enemy1PATH "../lib/resources/ufo.png"
#define hardMapBackgroundPATH "../lib/resources/map2Background.png"
#define hardMapImage1PATH "../lib/resources/map2Image1.png"
#define hardMapImage2PATH "../lib/resources/map2Image2.png"

typedef struct {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ship *pShips[MAX_PLAYERS];
    Cannon *pCannons[MAX_PLAYERS];
    int nrOfShips, shipId;
    GameState state;
    Mix_Music *pMusic;
    TTF_Font *pFont, *pSmallFont;
    Text *pSinglePlayerText, *pGameName, *pExitText, *pPauseText, *pTimer, *pMultiPlayerText,
        *pMenuText, *pGameOverText, *pWaitingText;
    ClientCommand command, lastCommand;
    UDPsocket pSocket;
    IPaddress serverAddress;
    UDPpacket *pPacket;
    bool isRunning, isShooting, spacePressed;
    Stars *pStars;
    SDL_Texture *pStartImage_1, *pStartImage_2, *pHardMapBackground, *pHardMapImage1,
        *pHardMapImage2;
    Text *pCountdownText;
    EnemyImage *pEnemy_1Image;
    Enemy *pEnemies_1[MAX_ENEMIES];
    int nrOfEnemies_1;
    Enemy_2 *pEnemies_2[MAX_ENEMIES];
    EnemyImage_2 *pEnemy_2Image;
    int nrOfEnemies_2;

    Enemy_3 *pEnemies_3[NROFBOSSES];
    EnemyImage_3 *pEnemy_3Image;
    int nrOfEnemies_3;

    ServerData serverData;
    int map;
    int gameTime;  // in s
    int startTime; // in ms

    bool keyHeld[SDL_NUM_SCANCODES]; // track all key states for smooth movement!
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void handleStartState(Game *pGame);
void renderStartWindow(Game *pGame);
void handleOngoingState(Game *pGame);
void handleLobbyState(Game *pGame);
void printMultiplayerMenu(Game *pGame, char *pEnteredIPAddress, bool textFieldFocused);
void handleGameOverState(Game *pGame);
void closeGame(Game *pGame);
void handleInput(SDL_Event *pEvent, Game *pGame);
bool connectToServer(Game *pGame);
void receiveDataFromServer();
void updateWithServerData(Game *pGame);
MainMenuChoice handleMainMenuOptions(Game *pGame);
void showCountdown(Game *pGame);
bool areTheyAllDead(Game *pGame);
void updateGameTime(Game *pGame);
int getTime(Game *pGame);
void drawMap(Game *pGame);
void drawMapTransitionScreen(SDL_Renderer *renderer, TTF_Font *pFont);
ClientCommand getCurrentCommand(Game *pGame);
void resetGameState(Game *pGame);

int main(int argc, char **argv) {
    Game game = {0};
    if (!initiate(&game)) {
        closeGame(&game);
        return 1;
    }
    printf("Entering run()\n");
    run(&game);
    printf("Left run()\n");
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
    pGame->pWindow = SDL_CreateWindow("Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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
    pGame->pFont = TTF_OpenFont("../lib/resources/arial.ttf", 100);
    pGame->pSmallFont = TTF_OpenFont("../lib/resources/arial.ttf", 50);
    if (!pGame->pFont || !pGame->pSmallFont) {
        printf("Error: %s\n", TTF_GetError());
        return 0;
    }
    pGame->pSinglePlayerText = createText(pGame->pRenderer, 255, 0, 0, pGame->pSmallFont,
                                          "Singleplayer", WINDOW_WIDTH / 2, 330);
    pGame->pMultiPlayerText = createText(pGame->pRenderer, 255, 0, 0, pGame->pSmallFont,
                                         "Multiplayer", WINDOW_WIDTH / 2, 450);
    pGame->pGameName = createText(pGame->pRenderer, 255, 0, 0, pGame->pFont, "Solar Defence",
                                  WINDOW_WIDTH / 2, WINDOW_HEIGHT / 8);
    pGame->pExitText =
        createText(pGame->pRenderer, 255, 0, 0, pGame->pSmallFont, "Exit", WINDOW_WIDTH / 2, 570);
    pGame->pWaitingText =
        createText(pGame->pRenderer, 255, 0, 0, pGame->pSmallFont,
                   "Waiting for other players to join...", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    if (!(pGame->pPacket = SDLNet_AllocPacket(5000))) {
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
    pGame->pStars = createStars(WINDOW_WIDTH * WINDOW_HEIGHT / 10000, WINDOW_WIDTH, WINDOW_HEIGHT);
    SDL_Surface *tempSurface_1 = IMG_Load(EARTHPATH);
    if (!tempSurface_1) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->pStartImage_1 = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface_1);
    SDL_FreeSurface(tempSurface_1);
    if (!pGame->pStartImage_1) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }
    SDL_Surface *tempSurface_2 = IMG_Load(ASTEROIDPATH);
    if (!tempSurface_2) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->pStartImage_2 = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface_2);
    SDL_FreeSurface(tempSurface_2);
    if (!pGame->pStartImage_2) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }

    SDL_Surface *tempSurface_3 = IMG_Load(hardMapBackgroundPATH);
    if (!tempSurface_3) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->pHardMapBackground = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface_3);
    SDL_FreeSurface(tempSurface_3);
    if (!pGame->pHardMapBackground) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }

    SDL_Surface *tempSurface_4 = IMG_Load(hardMapImage1PATH);
    if (!tempSurface_4) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->pHardMapImage1 = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface_4);
    SDL_FreeSurface(tempSurface_4);
    if (!pGame->pHardMapImage1) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }

    SDL_Surface *tempSurface_5 = IMG_Load(hardMapImage2PATH);
    if (!tempSurface_5) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->pHardMapImage2 = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface_5);
    SDL_FreeSurface(tempSurface_5);
    if (!pGame->pHardMapImage2) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }

    pGame->pEnemy_1Image = initiateEnemy(pGame->pRenderer);
    pGame->nrOfEnemies_1 = 0;
    pGame->pEnemy_2Image = initiateEnemy_2(pGame->pRenderer);
    pGame->nrOfEnemies_2 = 0;
    pGame->pEnemy_3Image = initiateEnemy_3(pGame->pRenderer);
    pGame->nrOfEnemies_3 = 0;

    memset(pGame->keyHeld, 0, sizeof(pGame->keyHeld));

    pGame->isRunning = true;
    pGame->state = START;
    pGame->isShooting = false;
    return 1;
}

void run(Game *pGame) {
    while (pGame->isRunning) {
        switch (pGame->state) {
        case START:
            handleStartState(pGame);
            break;
        case ONGOING:
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

void handleStartState(Game *pGame) {
    SDL_Event event;
    while (pGame->isRunning && pGame->state == START) {
        MainMenuChoice userChoice = handleMainMenuOptions(pGame);
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || userChoice == MAINMENU_EXIT) {
                pGame->isRunning = false;
                return;
            } else if (userChoice == MAINMENU_SINGLEPLAYER) {
                printf("Singleplayer chosen.\n");
            } else if (userChoice == MAINMENU_MULTIPLAYER) {
                printf("Multiplayer chosen.\n");
                pGame->state = LOBBY;
                return;
            }
        }
        renderStartWindow(pGame);
        SDL_Delay(32);
    }
}

void renderStartWindow(Game *pGame) {
    SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(pGame->pRenderer);
    SDL_SetRenderDrawColor(pGame->pRenderer, 255, 255, 255, 255); // White stars
    drawStars(pGame->pStars, pGame->pRenderer);
    drawText(pGame->pSinglePlayerText);
    drawText(pGame->pMultiPlayerText);
    drawText(pGame->pExitText);
    drawText(pGame->pGameName);

    SDL_Rect dstRect_1 = {125, 500, 100, 100}; // adjust position and size, placering av planet/måne
    SDL_RenderCopy(pGame->pRenderer, pGame->pStartImage_1, NULL, &dstRect_1);
    SDL_Rect dstRect_2 = {1000, 125, 50, 50}; // adjust position and size, placering av planet/måne
    SDL_RenderCopy(pGame->pRenderer, pGame->pStartImage_2, NULL, &dstRect_2);
    SDL_RenderPresent(pGame->pRenderer);
}

void handleOngoingState(Game *pGame) {
    SDL_Event event;
    ClientData cData;
    Uint32 now = 0, delta = 0, lastSend = 0, lastUpdate = SDL_GetTicks();
    const Uint32 tickInterval = 8, resendIntervall = 50;
    pGame->command = STOP_SHIP;
    pGame->lastCommand = STOP_SHIP;
    pGame->startTime = SDL_GetTicks64();
    pGame->gameTime = -1; // i dont know why
    pGame->map = 1;
    bool seenMapTransition = false;
    int nextMapShowWhen = 10;
    while (pGame->isRunning && pGame->state == ONGOING) {
        now = SDL_GetTicks();
        delta = now - lastUpdate; // används bara på rad 253, men delta används inte
        while (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
            updateWithServerData(pGame);
        }
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pGame->isRunning = false;
                return;
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP ||
                       event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
                handleInput(&event, pGame);
            }
        }

        if (timeToUpdate(&lastUpdate, tickInterval)) {
            pGame->command = getCurrentCommand(pGame);
            applyShipCommand(pGame->pShips[pGame->shipId], pGame->command);
            if (pGame->command != pGame->lastCommand || pGame->isShooting ||
                now - lastSend >= resendIntervall) { // Skicka endast om användare ändrar command ||
                ClientData ccData = {.command = pGame->command, .isShooting = pGame->isShooting};
                if (seenMapTransition == true) {
                    ccData.map = pGame->map;
                }
                memcpy(pGame->pPacket->data, &ccData, sizeof(ClientData));
                pGame->pPacket->len = sizeof(ClientData);
                SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
                pGame->lastCommand = pGame->command;
                lastSend = now;
                updateGameTime(pGame);
            }

            updateShipVelocity(pGame->pShips[pGame->shipId]);
            // if (pGame->isShooting) {
            //     handleCannonEvent(pGame->pCannons[pGame->shipId]);
            //     //pGame->isShooting = false;
            // }
            // Använder vi ens prediction?
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (pGame->pShips[i]) {
                    removeProjectile(getBulletToRemove(pGame->pShips[i]));
                    update_projectiles(delta);
                    updateShipOnClients(pGame->pShips[i], i,
                                        pGame->shipId); // <--- pass remote shipId and myShipId
                    updateCannon(pGame->pCannons[i], pGame->pShips[i]);
                    if (isCannonShooting(pGame->pShips[i])) {
                        handleCannonEvent(pGame->pCannons[i]);
                    }
                }
            }
            for (int i = 0; i < pGame->nrOfEnemies_1; i++) {
                pGame->pEnemies_1[i] =
                    createEnemyOnClient(pGame->pEnemy_1Image, WINDOW_WIDTH, WINDOW_HEIGHT,
                                        pGame->serverData.enemies_1[i]);
            }
            for (int i = 0; i < pGame->nrOfEnemies_2; i++) {
                pGame->pEnemies_2[i] =
                    createEnemy_2_OnClients(pGame->pEnemy_2Image, WINDOW_WIDTH, WINDOW_HEIGHT,
                                            pGame->serverData.enemies_2[i]);
            }
            for (int i = 0; i < pGame->nrOfEnemies_3; i++) {
                pGame->pEnemies_3[i] =
                    createEnemy_3_OnClients(pGame->pEnemy_3Image, WINDOW_WIDTH, WINDOW_HEIGHT,
                                            pGame->serverData.enemies_3[i]);
            }
            SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
            SDL_RenderClear(pGame->pRenderer);
            drawMap(pGame);
            if (pGame->pTimer) drawText(pGame->pTimer);
            if (pGame->gameTime >= nextMapShowWhen) pGame->map = 2;
            if (seenMapTransition == false && pGame->gameTime >= nextMapShowWhen - 1) {
                seenMapTransition = true;
                SDL_SetRenderDrawColor(pGame->pRenderer, 175, 0, 0, 255);
                drawMapTransitionScreen(pGame->pRenderer, pGame->pFont);
            }
            /*for (int i = 0; i < MAX_PLAYERS; i++) {
              render_projectiles(pGame->pRenderer);
              drawShip(pGame->pShips[i]);
              drawCannon(pGame->pCannons[i]);
            }*/
            for (int i = 0; i < pGame->nrOfEnemies_1; i++) {
                if (isEnemyActive(pGame->pEnemies_1[i])) {
                    updateEnemyOnClients(pGame->pEnemies_1[i], pGame->serverData.enemies_1[i]);
                    if (pGame->gameTime <= nextMapShowWhen - 1) {
                        drawEnemy(pGame->pEnemies_1[i]);
                    }
                }
            }
            for (int i = 0; i < pGame->nrOfEnemies_2; i++) {
                if (isEnemy_2Active(pGame->pEnemies_2[i])) {
                    updateEnemy_2_OnClients(pGame->pEnemies_2[i], pGame->serverData.enemies_2[i]);
                    drawEnemy_2(pGame->pEnemies_2[i]);
                }
            }
            for (int i = 0; i < pGame->nrOfEnemies_3; i++) {
                if (isEnemy_3Active(pGame->pEnemies_3[i]) && pGame->map == 2) {
                    updateEnemy_3_OnClients(pGame->pEnemies_3[i], pGame->serverData.enemies_3[i]);
                    drawEnemy_3(pGame->pEnemies_3[i]);
                }
            }
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (!clientAliveControll(pGame->pShips[i])) {
                    damageCannon(pGame->pCannons[i], 2);
                    damageShip(pGame->pShips[i], 2);
                }
                render_projectiles(pGame->pRenderer);
                drawShip(pGame->pShips[i]);
                drawCannon(pGame->pCannons[i]);
            }
            SDL_RenderPresent(pGame->pRenderer);
            pGame->isShooting = false;
            for (int i = 0; i < MAX_PLAYERS; i++) {
                setShoot(pGame->pShips[i], false);
            }
        }
    }
}

void handleLobbyState(Game *pGame) {
    SDL_Event event;
    bool socketOpened = false, textFieldFocused = false;
    SDL_StartTextInput();                  // Enable text input
    static char enteredIPAddress[32] = ""; // Buffer to store the entered string

    while (pGame->isRunning && pGame->state == LOBBY) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pGame->isRunning = false;
                return;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_Point click = {event.button.x, event.button.y};
                SDL_Rect inputBox = {300, 300, 600, 100};
                if (SDL_PointInRect(&click, &inputBox)) {
                    textFieldFocused = true;
                } else {
                    textFieldFocused = false;
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(enteredIPAddress) > 0) {
                    enteredIPAddress[strlen(enteredIPAddress) - 1] =
                        '\0'; // Remove the last character
                } else if (event.key.keysym.sym == SDLK_RETURN) {
                    printf("Entered IP: %s\n", enteredIPAddress);

                    // Attempt to resolve the entered IP address
                    if (SDLNet_ResolveHost(&(pGame->serverAddress), enteredIPAddress,
                                           SERVER_PORT) == 0) {
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
                            while (pGame->state != ONGOING) {
                                if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket) == 1) {
                                    if (strncmp((char *)pGame->pPacket->data, "ONGOING", 7) == 0) {
                                        // set a countdown
                                        showCountdown(pGame);
                                        pGame->state = ONGOING;
                                        return;
                                    }
                                }
                                SDL_SetRenderDrawColor(pGame->pRenderer, 30, 30, 30, 255);
                                SDL_RenderClear(pGame->pRenderer);
                                drawText(pGame->pWaitingText);
                                SDL_RenderPresent(pGame->pRenderer);
                                if (SDL_PollEvent(&event)) {
                                    if (event.type == SDL_QUIT) {
                                        pGame->isRunning = false;
                                        return;
                                    } else if (event.type == SDL_KEYDOWN &&
                                               event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                                        pGame->state = ONGOING;
                                        return;
                                    }
                                }
                                SDL_Delay(16);
                            }
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
                    strncat(enteredIPAddress, event.text.text,
                            sizeof(enteredIPAddress) - strlen(enteredIPAddress) -
                                1); // Append the entered character
                }
            }
        }
        printMultiplayerMenu(pGame, enteredIPAddress, textFieldFocused);
        SDL_Delay(32);
    }
    SDL_StopTextInput();
}

void printMultiplayerMenu(Game *pGame, char *pEnteredIPAddress, bool textFieldFocused) {
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
    SDL_Rect textRect = {box.x + 5, box.y + 10, 0, 0};
    SDL_Surface *textSurface = TTF_RenderText_Solid(pGame->pSmallFont, pEnteredIPAddress, color);

    if (textSurface) {
        textRect.w = textSurface->w;
        textRect.h = textSurface->h;

        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, textSurface);
        SDL_RenderCopy(pGame->pRenderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    } else {
        textRect.h = TTF_FontHeight(pGame->pSmallFont);
    }

    // Render the prompt text
    SDL_Surface *promptSurface1 =
        TTF_RenderText_Solid(pGame->pSmallFont, "Type in server IP ADDRESS and press ENTER", color);
    if (promptSurface1) {
        SDL_Texture *promptTexture1 =
            SDL_CreateTextureFromSurface(pGame->pRenderer, promptSurface1);
        SDL_Rect promptRect1 = {box.x - 150, box.y - 150, promptSurface1->w,
                                promptSurface1->h}; // Position above the input box
        SDL_RenderCopy(pGame->pRenderer, promptTexture1, NULL, &promptRect1);
        SDL_FreeSurface(promptSurface1);
        SDL_DestroyTexture(promptTexture1);
    }

    SDL_Surface *promptSurface2 =
        TTF_RenderText_Solid(pGame->pSmallFont, "Press ESCAPE to go to the MAIN MENU", color);
    if (promptSurface2) {
        SDL_Texture *promptTexture2 =
            SDL_CreateTextureFromSurface(pGame->pRenderer, promptSurface2);
        SDL_Rect promptRect2 = {box.x - 150, box.y + 150, promptSurface2->w,
                                promptSurface2->h}; // Position below the first line
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
        SDL_Rect caretRect = {caretX, caretY, 3, caretHeight};
        SDL_RenderFillRect(pGame->pRenderer, &caretRect);
    }
    SDL_RenderPresent(pGame->pRenderer);
}

void handleGameOverState(Game *pGame) {
    Text *pGameOverText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "GAME OVER",
                                     WINDOW_WIDTH / 2, 150);
    Text *pGameOverText2 = createText(pGame->pRenderer, 238, 168, 65, pGame->pSmallFont,
                                      "MAIN MENU", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    SDL_RenderPresent(pGame->pRenderer);

    const SDL_Rect *pReplayRect =
        getTextRect(pGameOverText2); // Hämta position för rect för Start-texten
    SDL_Event event;
    while (pGame->isRunning) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        SDL_Point mousePoint = {x, y}; // Kolla position för musen
        if (SDL_PointInRect(&mousePoint, pReplayRect)) {
            setTextColor(pGameOverText2, 255, 100, 100, pGame->pSmallFont, "MAIN MENU");
        } else {
            setTextColor(pGameOverText2, 238, 168, 65, pGame->pSmallFont, "MAIN MENU");
        }

        SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255); // Clear with black
        SDL_RenderClear(pGame->pRenderer);
        drawText(pGameOverText);
        drawText(pGameOverText2);
        SDL_RenderPresent(pGame->pRenderer);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pGame->isRunning = false;
            } else if (SDL_PointInRect(&mousePoint, pReplayRect) &&
                       event.type == SDL_MOUSEBUTTONDOWN) {
                printf("I dunno how to reset the whole game for everyone yet.");
                // closeGame(pGame);
                memset(pGame->keyHeld, 0,
                       sizeof(pGame->keyHeld)); // optional: zero client key state too
                pGame->command = STOP_SHIP;
                pGame->lastCommand = STOP_SHIP;
                resetGameState(pGame);
                pGame->state = START;
                return;
            }
        }
    }
}
void updateWithServerData(Game *pGame) {
    ServerData serverData; /////// test
    memcpy(&serverData, pGame->pPacket->data, sizeof(ServerData));
    pGame->shipId = serverData.sDPlayerId; ////// test  Kontrollera varför vi gör detta
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (pGame->pShips[i])
            updateShipsWithServerData(pGame->pShips[i], &serverData.ships[i], i, pGame->shipId);
    }
    pGame->nrOfEnemies_1 = serverData.nrOfEnemies_1;
    pGame->nrOfEnemies_2 = serverData.nrOfEnemies_2;
    pGame->nrOfEnemies_3 = serverData.nrOfEnemies_3;
    pGame->state = serverData.gState;
    pGame->serverData = serverData;
}

bool connectToServer(Game *pGame) {
    const char *msg = "TRYING TO CONNECT";
    memcpy(pGame->pPacket->data, msg, strlen(msg) + 1);
    pGame->pPacket->len = strlen(msg) + 1;
    pGame->pPacket->address = pGame->serverAddress;
    if (SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket) == 0) {
        printf("Failed to send packet: %s\n", SDLNet_GetError());
    } else {
        printf("Packet sent to server.\n");
    }
    bool connected = false;
    Uint32 startTime = SDL_GetTicks(); // Get the current time in milliseconds
    const Uint32 timeout = 5000;       // Set a timeout of 5000ms (5 seconds)
    while (!connected) {
        if (SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) {
            printf("*Received from server: %s\n", (char *)pGame->pPacket->data);
            ServerData serverData; /////// test
            memcpy(&serverData, pGame->pPacket->data, sizeof(ServerData));
            pGame->shipId = serverData.sDPlayerId; ////// test
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

void handleInput(SDL_Event *pEvent, Game *pGame) {
    SDL_Scancode key = pEvent->key.keysym.scancode;
    if (pEvent->type == SDL_KEYDOWN || pEvent->type == SDL_KEYUP) {
        bool isDown = (pEvent->type == SDL_KEYDOWN);
        pGame->keyHeld[key] = isDown;

        if (key == SDL_SCANCODE_SPACE) {
            if (isDown) {
                pGame->spacePressed = true;
            } else if (pGame->spacePressed) {
                pGame->isShooting = true;
                pGame->spacePressed = false;
            }
        }
    } else if (pEvent->type == SDL_MOUSEBUTTONDOWN && pEvent->button.button == SDL_BUTTON_LEFT) {
        pGame->spacePressed = true;
    } else if (pEvent->type == SDL_MOUSEBUTTONUP && pEvent->button.button == SDL_BUTTON_LEFT &&
               pGame->spacePressed) {
        pGame->isShooting = true;
        pGame->spacePressed = false;
    }
}

ClientCommand getCurrentCommand(Game *pGame) {
    bool up = pGame->keyHeld[SDL_SCANCODE_W] || pGame->keyHeld[SDL_SCANCODE_UP];
    bool down = pGame->keyHeld[SDL_SCANCODE_S] || pGame->keyHeld[SDL_SCANCODE_DOWN];
    bool left = pGame->keyHeld[SDL_SCANCODE_A] || pGame->keyHeld[SDL_SCANCODE_LEFT];
    bool right = pGame->keyHeld[SDL_SCANCODE_D] || pGame->keyHeld[SDL_SCANCODE_RIGHT];

    if (up && left) return MOVE_UP_LEFT;
    if (up && right) return MOVE_UP_RIGHT;
    if (down && left) return MOVE_DOWN_LEFT;
    if (down && right) return MOVE_DOWN_RIGHT;
    if (up) return MOVE_UP;
    if (down) return MOVE_DOWN;
    if (left) return MOVE_LEFT;
    if (right) return MOVE_RIGHT;

    return STOP_SHIP;
}

MainMenuChoice handleMainMenuOptions(Game *pGame) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    SDL_Point mousePoint = {x, y};

    const SDL_Rect *singleRect = getTextRect(pGame->pSinglePlayerText);
    const SDL_Rect *multiRect = getTextRect(pGame->pMultiPlayerText);
    const SDL_Rect *exitRect = getTextRect(pGame->pExitText);

    if (SDL_PointInRect(&mousePoint, singleRect))
        setTextColor(pGame->pSinglePlayerText, 255, 255, 255, pGame->pSmallFont, "Singleplayer");
    else
        setTextColor(pGame->pSinglePlayerText, 255, 0, 0, pGame->pSmallFont, "Singleplayer");

    if (SDL_PointInRect(&mousePoint, multiRect))
        setTextColor(pGame->pMultiPlayerText, 255, 255, 255, pGame->pSmallFont, "Multiplayer");
    else
        setTextColor(pGame->pMultiPlayerText, 255, 0, 0, pGame->pSmallFont, "Multiplayer");

    if (SDL_PointInRect(&mousePoint, exitRect))
        setTextColor(pGame->pExitText, 255, 255, 255, pGame->pSmallFont, "Exit");
    else
        setTextColor(pGame->pExitText, 255, 0, 0, pGame->pSmallFont, "Exit");

    Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
    if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (SDL_PointInRect(&mousePoint, singleRect)) return MAINMENU_SINGLEPLAYER;
        if (SDL_PointInRect(&mousePoint, multiRect)) return MAINMENU_MULTIPLAYER;
        if (SDL_PointInRect(&mousePoint, exitRect)) return MAINMENU_EXIT;
    }
    return MAINMENU_NONE;
}

void showCountdown(Game *pGame) {
    int countdown = COUNTDOWN;
    char buffer[16];
    char joinMsg[64];

    snprintf(joinMsg, sizeof(joinMsg), "%d players have joined. Starting game in...", MAX_PLAYERS);
    Text *pJoinText = createText(pGame->pRenderer, 255, 0, 0, pGame->pSmallFont, joinMsg,
                                 WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 100);
    Uint32 lastTick = SDL_GetTicks();

    while (countdown > 0 && pGame->isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                pGame->isRunning = false;
                break;
            }
        }

        Uint32 now = SDL_GetTicks();
        if (now - lastTick >= 1000) {
            lastTick = now;

            snprintf(buffer, sizeof(buffer), "%d", countdown);
            if (pGame->pCountdownText) destroyText(pGame->pCountdownText);
            pGame->pCountdownText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont,
                                               buffer, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

            SDL_SetRenderDrawColor(pGame->pRenderer, 30, 30, 30, 255);
            SDL_RenderClear(pGame->pRenderer);
            drawText(pJoinText);
            drawText(pGame->pCountdownText);
            SDL_RenderPresent(pGame->pRenderer);

            countdown--;
        }
        SDL_Delay(16);
    }

    if (pGame->pCountdownText) {
        destroyText(pGame->pCountdownText);
        pGame->pCountdownText = NULL;
    }
    if (pJoinText) {
        destroyText(pJoinText);
    }
}

void closeGame(Game *pGame) {
    printf("Entering closeGame()\n");
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (pGame->pShips[i]) destroyShip(pGame->pShips[i]);
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (pGame->pCannons[i]) destroyCannon(pGame->pCannons[i]);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);

    if (pGame->pGameName) destroyText(pGame->pGameName);
    if (pGame->pSinglePlayerText) destroyText(pGame->pSinglePlayerText);
    if (pGame->pMultiPlayerText) destroyText(pGame->pMultiPlayerText);
    if (pGame->pExitText) destroyText(pGame->pExitText);
    if (pGame->pWaitingText) destroyText(pGame->pWaitingText);
    if (pGame->pPauseText) destroyText(pGame->pPauseText);
    if (pGame->pTimer) destroyText(pGame->pTimer);
    if (pGame->pMenuText) destroyText(pGame->pMenuText);
    if (pGame->pGameOverText) destroyText(pGame->pGameOverText);
    if (pGame->pCountdownText) destroyText(pGame->pCountdownText);

    if (pGame->pStars) destroyStars(pGame->pStars);
    if (pGame->pStartImage_1) SDL_DestroyTexture(pGame->pStartImage_1);
    if (pGame->pStartImage_2) SDL_DestroyTexture(pGame->pStartImage_2);
    if (pGame->pHardMapBackground) SDL_DestroyTexture(pGame->pHardMapBackground);
    if (pGame->pHardMapImage1) SDL_DestroyTexture(pGame->pHardMapImage1);
    if (pGame->pHardMapImage2) SDL_DestroyTexture(pGame->pHardMapImage2);

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

int getTime(Game *pGame) {
    return (SDL_GetTicks64() - pGame->startTime) / 1000;
}

void updateGameTime(Game *pGame) {
    if (getTime(pGame) > pGame->gameTime && pGame->state == ONGOING) {
        (pGame->gameTime)++;
        if (pGame->pTimer) destroyText(pGame->pTimer);
        static char timerString[30];
        sprintf(timerString, "%d", getTime(pGame));
        if (pGame->pSmallFont) {
            pGame->pTimer = createText(pGame->pRenderer, 238, 168, 65, pGame->pSmallFont,
                                       timerString, WINDOW_WIDTH / 2, 50);
        }
    }
}

void drawMap(Game *pGame) {
    if (pGame->map == 1) {
        SDL_SetRenderDrawColor(pGame->pRenderer, 255, 255, 255,
                               255); // Set star color to white if they arent already
        drawStars(pGame->pStars, pGame->pRenderer);
        SDL_Rect earthImageRect = {WINDOW_WIDTH / 2.5, WINDOW_HEIGHT / 3, 200, 200};
        SDL_RenderCopy(pGame->pRenderer, pGame->pStartImage_1, NULL, &earthImageRect);
    } else if (pGame->map == 2) {
        SDL_SetRenderDrawColor(pGame->pRenderer, 255, 0, 0,
                               255); // Set star color to red if they arent already
        drawStars(pGame->pStars, pGame->pRenderer);
        SDL_Rect backgroundRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderCopy(pGame->pRenderer, pGame->pHardMapBackground, NULL, &backgroundRect);
        drawStars(pGame->pStars, pGame->pRenderer);
        SDL_Rect centerImageRect = {WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3.5, 128 * 3, 97 * 3};
        SDL_RenderCopy(pGame->pRenderer, pGame->pHardMapImage1, NULL, &centerImageRect);
        SDL_Rect cornerImageRect = {100, 65, 48, 48};
        SDL_RenderCopy(pGame->pRenderer, pGame->pHardMapImage2, NULL, &cornerImageRect);
    }
}

void drawMapTransitionScreen(SDL_Renderer *renderer,
                             TTF_Font *pFont) { // assumes a rendercolor is chosen before
    Text *pChangeMapText =
        createText(renderer, 238, 168, 65, pFont, "Earth Defended!", WINDOW_WIDTH / 2, 250);
    Text *pChangeMapText2 = createText(renderer, 238, 168, 65, pFont, "Attack The Alien Planet!",
                                       WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    drawText(pChangeMapText);
    drawText(pChangeMapText2);
    SDL_RenderPresent(renderer); // draw whole screen
    SDL_Delay(4000);             // Delay for like a sec
}

void resetGameState(Game *pGame) {
    // Reset player ships and cannons
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (pGame->pShips[i]) destroyShip(pGame->pShips[i]);
    for (int i = 0; i < MAX_PLAYERS; i++)                          //
        if (pGame->pCannons[i]) destroyCannon(pGame->pCannons[i]); //
    // if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
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
    // Reset counts and state
    pGame->map = 1;
    resetAllBullets();
}