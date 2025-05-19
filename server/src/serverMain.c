#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_net.h>
#include <time.h>

#include "../../lib/include/bullet.h"
#include "../../lib/include/cannon.h"
#include "../../lib/include/enemy_1.h"
#include "../../lib/include/enemy_2.h"
#include "../../lib/include/enemy_3.h"
#include "../../lib/include/ship.h"
#include "../../lib/include/ship_data.h"
#include "../../lib/include/sound.h"
#include "../../lib/include/text.h"
#include "../../lib/include/tick.h"

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
    TTF_Font *pFont;
    Text *pStartText, *pGameName, *pExitText, *pLobbyText;
    ClientCommand command;
    IPaddress clients[MAX_PLAYERS];
    UDPsocket pSocket;
    UDPpacket *pPacket;
    ServerData serverData;
    bool isRunning, isShooting;
    Cannon *pCannon;
    int nrOfEnemiesToSpawn_1, nrOfEnemies_1, killedEnemies, nrOfEnemiesToSpawn_2, nrOfEnemies_2;
    EnemyImage *pEnemy_1Image;
    EnemyImage_2 *pEnemy_2Image;
    Enemy *pEnemies_1[MAX_ENEMIES];
    Enemy_2 *pEnemies_2[MAX_ENEMIES];

    EnemyImage_3 *pEnemy_3Image;
    Enemy_3 *pEnemies_3[1];
    int nrOfEnemies_3, nrOfEnemiesToSpawn_3;

    int map;
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
bool areAllEnemy_1_Dead(Game *pGame);
void printPacketsData(Game *pGame);
void spawnEnemies_2(Game *pGame, int amount);
void updateEnemies_2(Game *pGame, int *amount);
bool areAllEnemy_2_Dead(Game *pGame);
void spawnBoss(Game *pGame);
void resetBoss(Game *pGame);
void updateBoss(Game *pGame);
void printPacketsData(Game *pGame);

int main(int argc, char **argv) {
    Game game = {0};
    printf("doing intiate\n");
    if (!initiate(&game)) {
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
        return -1;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        printf("SDL_image Init Error: %s\n", IMG_GetError());
        return -1;
    }
    if (TTF_Init() != 0) {
        printf("Error: %s\n", TTF_GetError());
        return -1;
    }
    if (SDLNet_Init()) {
        printf("SDLNet_Init: %s\n", SDLNet_GetError());
        return -1;
    }
    pGame->pWindow =
        SDL_CreateWindow("Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!pGame->pWindow) {
        printf("Window Error: %s\n", SDL_GetError());
        return -1;
    }
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        return -1;
    }
    pGame->pFont = TTF_OpenFont("../lib/resources/arial.ttf", 100);
    if (!pGame->pFont) {
        printf("Error: %s\n", TTF_GetError());
        return -1;
    }
    pGame->pStartText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Start [1]", WINDOW_WIDTH / 3,
                                   WINDOW_HEIGHT / 2 + 100);
    pGame->pGameName =
        createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "SpaceShooter", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 4);
    pGame->pExitText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Exit [8]", WINDOW_WIDTH / 1.5,
                                  WINDOW_HEIGHT / 2 + 100);
    pGame->pLobbyText = createText(pGame->pRenderer, 238, 168, 65, pGame->pFont, "Waiting on clients...",
                                   WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    if (!pGame->pFont) {
        printf("Error: %s\n", TTF_GetError());
        return -1;
    }
    if (!(pGame->pSocket = SDLNet_UDP_Open(SERVER_PORT))) {
        printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return -1;
    }
    if (!(pGame->pPacket = SDLNet_AllocPacket(5120))) {
        printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return -1;
    }

    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
        pGame->pShips[playerIndex] = createShip(playerIndex, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
        pGame->pCannons[playerIndex] = createCannon(pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    pGame->nrOfShips = MAX_PLAYERS;
    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
        if (!pGame->pShips[playerIndex] || !pGame->pCannons[playerIndex]) {
            printf("Error: %s\n", SDL_GetError());
            return -1;
        }
    }
    if (!initMusic(&pGame->pMusic, MUSIC_FILEPATH)) {
        printf("Error: %s\n", Mix_GetError());
        return -1;
    }
    pGame->pEnemy_1Image = initiateEnemy(pGame->pRenderer);
    pGame->pEnemy_2Image = initiateEnemy_2(pGame->pRenderer);
    pGame->pEnemy_3Image = initiateEnemy_3(pGame->pRenderer);

    pGame->nrOfEnemies_1 = 0;
    pGame->nrOfEnemies_2 = 0;
    pGame->nrOfEnemies_3 = 0;

    pGame->isRunning = true;
    pGame->state = START;
    return 1;
}

void run(Game *pGame) {
    printf("Server is listening on port %hu...\n", SERVER_PORT);
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
                } else if (event.key.keysym.sym == SDLK_8) {
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
    Uint32 now = 0, delta = 0, lastUpdate = SDL_GetTicks();
    const Uint32 tickInterval = 16;
    pGame->nrOfEnemies_1 = 0;
    pGame->nrOfEnemies_2 = 0;
    SDL_Rect emptyRect = {0, 0, 0, 0}, rectArray[MAX_PROJECTILES] = {0, 0, 0, 0};

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
            pGame->map = cData.map; // idk if this is safe.
            if (clientIndex >= 0 && clientIndex < MAX_PLAYERS) {
                applyShipCommand(pGame->pShips[clientIndex], cData.command);
                if (cData.isShooting) {
                    setShoot(pGame->pShips[clientIndex], true);
                    // pGame->pShips[clientIndex]->shoot = true;
                }
            }
        }
        /*if (delta >= tickInterval) {
            lastUpdate = now;*/
        if (timeToUpdate(&lastUpdate, tickInterval)) {
            for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
                if (pGame->pShips[playerIndex]) {
                    // if (isShooting(pGame->pShips[i]) ) {
                    if (isCannonShooting(pGame->pShips[playerIndex])) {
                        printf("IN IS SHOOTING\n");
                        handleCannonEvent(pGame->pCannons[playerIndex]);
                        // setShoot(pGame->pShips[i], false);
                    }
                    update_projectiles(delta);
                    updateShipVelocity(pGame->pShips[playerIndex]);
                    updateShipOnServer(pGame->pShips[playerIndex]);
                    updateCannon(pGame->pCannons[playerIndex], pGame->pShips[playerIndex]);
                }
            }
            updateEnemies_1(pGame,
                            &pGame->nrOfEnemiesToSpawn_1); // Calls spawnEnemy() down at the bottom of the code
            updateEnemies_2(pGame, &pGame->nrOfEnemiesToSpawn_2);

            updateBoss(pGame);
            // updateEnemy_3(pGame->pEnemies_3[0]);
            for (int enemy1Index = 0; enemy1Index < pGame->nrOfEnemies_1 && enemy1Index < MAX_ENEMIES; enemy1Index++)
                updateEnemy(pGame->pEnemies_1[enemy1Index]); // locally

            for (int enemy2Index = 0; enemy2Index < pGame->nrOfEnemies_2 && enemy2Index < MAX_ENEMIES; enemy2Index++) {
                updateEnemy_2(pGame->pEnemies_2[enemy2Index]);
            }
            SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
            SDL_RenderClear(pGame->pRenderer);
            for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
                render_projectiles(pGame->pRenderer);
                drawShip(pGame->pShips[playerIndex]);
                drawCannon(pGame->pCannons[playerIndex]);
                // draw cannon
            }
            for (int enemy1Index = 0; enemy1Index < pGame->nrOfEnemies_1 && enemy1Index < MAX_ENEMIES; enemy1Index++) {
                if (isEnemyActive(pGame->pEnemies_1[enemy1Index]) == true) { // locally
                    drawEnemy(pGame->pEnemies_1[enemy1Index]);
                }
            }
            for (int enemy2Index = 0; enemy2Index < pGame->nrOfEnemies_2 && enemy2Index < MAX_ENEMIES; enemy2Index++) {
                if (isEnemy_2Active(pGame->pEnemies_2[enemy2Index])) drawEnemy_2(pGame->pEnemies_2[enemy2Index]);
            }
            if (isEnemy_3Active(pGame->pEnemies_3[0]) == true) {
                drawEnemy_3(pGame->pEnemies_3[0]);
            }
            // Ship collision här ??
            for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
                for (int enemy1Index = 0; enemy1Index < pGame->nrOfEnemies_1 && enemy1Index < MAX_ENEMIES;
                     enemy1Index++) {
                    // might change damage functions to avoid hard coded values
                    if (shipCollision(pGame->pShips[playerIndex], getRectEnemy(pGame->pEnemies_1[enemy1Index]))) {
                        damageEnemy(pGame->pEnemies_1[enemy1Index], 2, enemy1Index);
                        damageShip(pGame->pShips[playerIndex], 1);
                        damageCannon(pGame->pCannons[playerIndex], 1);
                        if (isPlayerDead(pGame->pShips[playerIndex])) {
                            printf("Player %d is dead\n", playerIndex);
                        }
                    }
                }
                for (int enemy2Index = 0; enemy2Index < pGame->nrOfEnemies_2 && enemy2Index < MAX_ENEMIES;
                     enemy2Index++) {
                    if (shipCollision(pGame->pShips[playerIndex], getRectEnemy_2(pGame->pEnemies_2[enemy2Index]))) {
                        damageEnemy_2(pGame->pEnemies_2[enemy2Index], 2, enemy2Index);
                        damageShip(pGame->pShips[playerIndex], 1);
                        damageCannon(pGame->pCannons[playerIndex], 1);
                        if (isPlayerDead(pGame->pShips[playerIndex])) {
                            printf("Player %d is dead\n", playerIndex);
                        }
                    }
                }
            }

            // Bullet collision här ?? Ska man kunna skjuta på varandra?
            getProjectileRects(rectArray);
            for (int spawnedBullet = 0; spawnedBullet < MAX_PROJECTILES; spawnedBullet++) {
                SDL_Rect bulletRect = rectArray[spawnedBullet];
                for (int enemy1Index = 0; enemy1Index < pGame->nrOfEnemies_1; enemy1Index++) {
                    SDL_Rect enemyRect = getRectEnemy(pGame->pEnemies_1[enemy1Index]);
                    if (SDL_HasIntersection(&enemyRect, &bulletRect)) {
                        printEnemyHealth(pGame->pEnemies_1[enemy1Index]);
                        damageEnemy(pGame->pEnemies_1[enemy1Index], 1, enemy1Index);
                        if (isEnemyActive(pGame->pEnemies_1[enemy1Index]) == false) {
                            (pGame->killedEnemies)++;
                        }
                        removeProjectile(spawnedBullet);
                        rectArray[spawnedBullet] = emptyRect;
                        for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
                            setBulletToRemove(pGame->pShips[playerIndex], spawnedBullet);
                        }
                    }
                }
                for (int enemy2Index = 0; enemy2Index < pGame->nrOfEnemies_2; enemy2Index++) {
                    SDL_Rect enemyRect2 = getRectEnemy_2(pGame->pEnemies_2[enemy2Index]);
                    if (SDL_HasIntersection(&enemyRect2, &bulletRect)) {
                        printEnemy_2Health(pGame->pEnemies_2[enemy2Index]);
                        damageEnemy_2(pGame->pEnemies_2[enemy2Index], 1, enemy2Index);
                        if (isEnemy_2Active(pGame->pEnemies_2[enemy2Index]) == false) {
                            (pGame->killedEnemies)++;
                        }
                        removeProjectile(spawnedBullet);
                        rectArray[spawnedBullet] = emptyRect;
                        for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
                            setBulletToRemove(pGame->pShips[playerIndex], spawnedBullet);
                        }
                    }
                }
            }
        }
        // Ship collision här ??
        for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
            for (int enemy1Index = 0; enemy1Index < pGame->nrOfEnemies_1 && enemy1Index < MAX_ENEMIES; enemy1Index++) {
                if (shipCollision(pGame->pShips[playerIndex], getRectEnemy(pGame->pEnemies_1[enemy1Index]))) {
                    damageEnemy(pGame->pEnemies_1[enemy1Index], 2, enemy1Index);
                    damageShip(pGame->pShips[playerIndex], 1);
                    damageCannon(pGame->pCannons[playerIndex], 1);
                    if (isPlayerDead(pGame->pShips[playerIndex])) {
                        printf("Player %d is dead\n", playerIndex);
                    }
                }
            }
            for (int enemy2Index = 0; enemy2Index < pGame->nrOfEnemies_2 && enemy2Index < MAX_ENEMIES; enemy2Index++) {
                if (shipCollision(pGame->pShips[playerIndex], getRectEnemy_2(pGame->pEnemies_2[enemy2Index]))) {
                    damageEnemy_2(pGame->pEnemies_2[enemy2Index], 2, enemy2Index);
                    damageShip(pGame->pShips[playerIndex], 1);
                    damageCannon(pGame->pCannons[playerIndex], 1);
                    if (isPlayerDead(pGame->pShips[playerIndex])) {
                        printf("Player %d is dead\n", playerIndex);
                    }
                }
            }
            for (int bossIndex = 0; bossIndex < pGame->nrOfEnemies_3 && bossIndex < MAX_ENEMIES; bossIndex++) {
                if (shipCollision(pGame->pShips[playerIndex], getRectEnemy_3(pGame->pEnemies_3[bossIndex]))) {
                    damageEnemy_3(pGame->pEnemies_3[bossIndex], 2, bossIndex);
                    damageShip(pGame->pShips[playerIndex], 1);
                    damageCannon(pGame->pCannons[playerIndex], 1);
                    if (isPlayerDead(pGame->pShips[playerIndex])) {
                        printf("Player %d is dead\n", playerIndex);
                    }
                }
            }
        }
        // Bullet collision här ?? Ska man kunna skjuta på varandra?
        getProjectileRects(rectArray);
        for (int bulletIndex = 0; bulletIndex < MAX_PROJECTILES; bulletIndex++) {
            SDL_Rect bulletRect = rectArray[bulletIndex];
            for (int enemy1Index = 0; enemy1Index < pGame->nrOfEnemies_1; enemy1Index++) {
                SDL_Rect enemyRect = getRectEnemy(pGame->pEnemies_1[enemy1Index]);
                if (SDL_HasIntersection(&enemyRect, &bulletRect)) {
                    printEnemyHealth(pGame->pEnemies_1[enemy1Index]);
                    damageEnemy(pGame->pEnemies_1[enemy1Index], 1, enemy1Index);
                    if (isEnemyActive(pGame->pEnemies_1[enemy1Index]) == false) {
                        (pGame->killedEnemies)++;
                    }
                    removeProjectile(bulletIndex);
                    rectArray[bulletIndex] = emptyRect;
                    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
                        setBulletToRemove(pGame->pShips[playerIndex], bulletIndex);
                    }
                }
            }
            for (int enemy2Index = 0; enemy2Index < pGame->nrOfEnemies_2; enemy2Index++) {
                SDL_Rect enemyRect2 = getRectEnemy_2(pGame->pEnemies_2[enemy2Index]);
                if (SDL_HasIntersection(&enemyRect2, &bulletRect)) {
                    printEnemy_2Health(pGame->pEnemies_2[enemy2Index]);
                    damageEnemy_2(pGame->pEnemies_2[enemy2Index], 1, enemy2Index);
                    if (isEnemy_2Active(pGame->pEnemies_2[enemy2Index]) == false) {
                        (pGame->killedEnemies)++;
                    }
                    removeProjectile(bulletIndex);
                    rectArray[bulletIndex] = emptyRect;
                    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
                        setBulletToRemove(pGame->pShips[playerIndex], bulletIndex);
                    }
                }
            }
            for (int bossIndex = 0; bossIndex < pGame->nrOfEnemies_3; bossIndex++) {
                SDL_Rect enemyRect3 = getRectEnemy_3(pGame->pEnemies_3[bossIndex]);
                if (SDL_HasIntersection(&enemyRect3, &bulletRect)) {
                    printEnemy_3Health(pGame->pEnemies_3[bossIndex]);
                    damageEnemy_3(pGame->pEnemies_3[bossIndex], 1, bossIndex);
                    if (isEnemy_3Active(pGame->pEnemies_3[bossIndex]) == false) {
                        (pGame->killedEnemies)++;
                    }
                    removeProjectile(bulletIndex);
                    rectArray[bulletIndex] = emptyRect;
                    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
                        setBulletToRemove(pGame->pShips[playerIndex], bulletIndex);
                    }
                }
            }
        }

        SDL_RenderPresent(pGame->pRenderer);
        sendServerData(pGame);
        for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
            setShoot(pGame->pShips[playerIndex], false);
            setBulletToRemove(pGame->pShips[playerIndex], -1);
        }
    }
}

void handleLobbyState(Game *pGame) {
    SDL_Event event;
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
            if (pGame->nrOfClients == MAX_PLAYERS) {
                pGame->state = ONGOING;
                for (int player = 0; player < MAX_PLAYERS; player++) {
                    const char *msg = "ONGOING";
                    memcpy(pGame->pPacket->data, msg, strlen(msg) + 1);
                    pGame->pPacket->address = pGame->clients[player];
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
    // Check if it's a connection request
    /*Uint32 ip = SDL_SwapBE32(pGame->pPacket->address.host);
    Uint16 port = SDL_SwapBE16(pGame->pPacket->address.port);
    printf("Received packet from %d.%d.%d.%d:%d\n", (ip >> 24) & 0xFF,
                        (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF, port);*/
    if (strncmp((char *)pGame->pPacket->data, "TRYING TO CONNECT", 17) == 0) {
        printf("Received connection request.\n");
        int clientIndex = getClientIndex(pGame, &pGame->pPacket->address);
        if (clientIndex >= 0 && clientIndex < MAX_PLAYERS) {
            // Respond with a client number
            pGame->serverData.sDPlayerId = clientIndex;
            memcpy(pGame->pPacket->data, &pGame->serverData, sizeof(ServerData));
            pGame->pPacket->len = sizeof(ServerData);
            pGame->pPacket->address = pGame->clients[clientIndex];
            // Printing packets to show what is being sent to the client *NEW*
            if (SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket)) {
                printf("Sent connection confirmation to client %d. \n", clientIndex);
                printf("*Sending first packet to client %d:\n", clientIndex);
                printPacketsData(pGame);
            } else
                printf("Failed to Send connection confirmation to client %d.\n", clientIndex);
        }
    }
}

/*
void handleGameOverState(Game *pGame) {

}*/

void sendServerData(Game *pGame) {
    if (pGame->nrOfClients == 0) return; // No clients connected
    pGame->serverData.gState = pGame->state;
    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++)
        getShipDataPackage(pGame->pShips[playerIndex], &pGame->serverData.ships[playerIndex]);

    for (int enemy1Index = 0; enemy1Index < pGame->nrOfEnemies_1 && enemy1Index < MAX_ENEMIES; enemy1Index++)
        getEnemy_1_DataPackage(pGame->pEnemies_1[enemy1Index], &pGame->serverData.enemies_1[enemy1Index]);
    pGame->serverData.nrOfEnemies_1 = pGame->nrOfEnemies_1;
    for (int enemy2Index = 0; enemy2Index < pGame->nrOfEnemies_2 && enemy2Index < MAX_ENEMIES; enemy2Index++)
        getEnemy_2_DataPackage(pGame->pEnemies_2[enemy2Index], &pGame->serverData.enemies_2[enemy2Index]);
    pGame->serverData.nrOfEnemies_2 = pGame->nrOfEnemies_2;
    for (int bossIndex = 0; bossIndex < pGame->nrOfEnemies_3 && bossIndex < NROFBOSSES; bossIndex++)
        getEnemy_3_DataPackage(pGame->pEnemies_3[bossIndex], &pGame->serverData.enemies_3[bossIndex]);
    pGame->serverData.nrOfEnemies_3 = pGame->nrOfEnemies_3;

    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
        pGame->serverData.sDPlayerId = playerIndex;
        memcpy(pGame->pPacket->data, &(pGame->serverData), sizeof(ServerData));
        pGame->pPacket->len = sizeof(ServerData);
        pGame->pPacket->address = pGame->clients[playerIndex];
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

    // Print each connected player's ship data
    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; playerIndex++) {
        printf("  Ship %d: x = %.2f, y = %.2f, angle = %.2f, alive = %d\n", playerIndex, sd->ships[playerIndex].x,
               sd->ships[playerIndex].y, sd->ships[playerIndex].vx, sd->ships[playerIndex].vy);
    }
    // Print enemy count
    printf("  Enemies_1: %d active, %d to spawn\n", sd->nrOfEnemies_1, sd->nrOfEnemiesToSpawn_1);
    // Print each enemy_1's data
    for (int enemy1Index = 0; enemy1Index < sd->nrOfEnemies_1; enemy1Index++) {
        printf("    Enemy_1[%d]: x = %.2f, y = %.2f, alive = %d\n", enemy1Index, sd->enemies_1[enemy1Index].x,
               sd->enemies_1[enemy1Index].y, sd->enemies_1[enemy1Index].active);
    }
    /*printf("  Enemies_2: %d active, %d to spawn\n", sd->nrOfEnemies_2,
    sd->nrOfEnemiesToSpawn_2); Print each enemy_2's data for (int i = 0; i <
    sd->nrOfEnemies_2; i++) { printf("    Enemy_1[%d]: x = %.2f, y = %.2f, alive =
    %d\n", i, sd->enemies_1[i].x, sd->enemies_2[i].y, sd->enemies_2[i].active);
    }*/
}

int getClientIndex(Game *pGame, IPaddress *clientAddr) {
    for (int playerIndex = 0; playerIndex < pGame->nrOfClients; playerIndex++) {
        if (pGame->clients[playerIndex].host == clientAddr->host &&
            pGame->clients[playerIndex].port == clientAddr->port) {
            return playerIndex; // Existing client
        }
    }
    if (pGame->nrOfClients < MAX_PLAYERS) {
        // New client
        pGame->clients[pGame->nrOfClients] = *clientAddr;
        for (int playerIndex = 0; playerIndex < pGame->nrOfClients; playerIndex++) {
            Uint32 ip = SDL_SwapBE32(pGame->clients[playerIndex].host);
            Uint16 port = SDL_SwapBE16(pGame->clients[playerIndex].port);
            printf("Client %d: %d.%d.%d.%d:%d\n", playerIndex, (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF,
                   ip & 0xFF, port);
        }
        return pGame->nrOfClients++;
    }
    return TOO_MANY_CLIENTS;
}

void closeGame(Game *pGame) {
    for (int clientIndex = 0; clientIndex < MAX_PLAYERS; clientIndex++)
        if (pGame->pShips[clientIndex]) destroyShip(pGame->pShips[clientIndex]);
    for (int clientIndex = 0; clientIndex < MAX_PLAYERS; clientIndex++)
        if (pGame->pCannons[clientIndex]) destroyCannon(pGame->pCannons[clientIndex]);
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

    for (int enemy1Index = 0; enemy1Index < MAX_ENEMIES; enemy1Index++)
        if (pGame->pEnemies_1[enemy1Index]) destroyEnemy_1(pGame->pEnemies_1[enemy1Index]);
    if (pGame->pEnemy_1Image) destroyEnemy_1Image(pGame->pEnemy_1Image);

    for (int enemy2Index = 0; enemy2Index < MAX_ENEMIES; enemy2Index++)
        if (pGame->pEnemies_2[enemy2Index]) destroyEnemy_2(pGame->pEnemies_2[enemy2Index]);
    if (pGame->pEnemy_2Image) destroyEnemyImage_2(pGame->pEnemy_2Image);

    for (int bossIndex = 0; bossIndex < NROFBOSSES; bossIndex++)
        if (pGame->pEnemies_3[bossIndex]) destroyEnemy_3(pGame->pEnemies_3[bossIndex]);
    if (pGame->pEnemy_3Image) destroyEnemyImage_3(pGame->pEnemy_3Image);
    Mix_CloseAudio();
    Mix_Quit();
    SDLNet_Quit();
    printf("SDL_NETQuit()\n");
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    printf("SDL_Quit");
}

void spawnEnemies_1(Game *pGame, int amount) {
    for (int spawnCount = 0; spawnCount < amount; spawnCount++) {
        pGame->pEnemies_1[pGame->nrOfEnemies_1] = createEnemy(pGame->pEnemy_1Image, WINDOW_WIDTH, WINDOW_HEIGHT);
        pGame->nrOfEnemies_1++;
    }
}

void updateEnemies_1(Game *pGame, int *amount) {
    if (areAllEnemy_1_Dead(pGame) == true) {
        if (pGame->map == 1) {
            (*amount) += 2;
        } else if (pGame->map == 2) {
            (*amount) += 10;
        }
        if ((*amount) > MAX_ENEMIES) (*amount) = MAX_ENEMIES;
        pGame->nrOfEnemies_1 = 0;
        spawnEnemies_1(pGame, *amount);
    }
}

bool areAllEnemy_1_Dead(Game *pGame) { // rename later to areAllEnemy_1_Dead
    for (int enemy1Index = 0; enemy1Index < pGame->nrOfEnemies_1 && enemy1Index < MAX_ENEMIES; enemy1Index++) {
        if (isEnemyActive(pGame->pEnemies_1[enemy1Index]) == true) {
            return false;
        }
    }
    return true;
}

void spawnEnemies_2(Game *pGame, int amount) {
    for (int spawnCount = 0; spawnCount < amount; spawnCount++) {
        pGame->pEnemies_2[pGame->nrOfEnemies_2] = createEnemy_2(pGame->pEnemy_2Image, WINDOW_WIDTH, WINDOW_HEIGHT);
        pGame->nrOfEnemies_2++;
    }
}

void updateEnemies_2(Game *pGame, int *amount) {
    if (areAllEnemy_2_Dead(pGame) == true) {
        (*amount) += 2; // increments even for first wave. WHY?
        if ((*amount) > MAX_ENEMIES) (*amount) = MAX_ENEMIES;
        pGame->nrOfEnemies_2 = 0;
        spawnEnemies_2(pGame, *amount);
    }
}

bool areAllEnemy_2_Dead(Game *pGame) {
    for (int enemy2Index = 0; enemy2Index < pGame->nrOfEnemies_2 && enemy2Index < MAX_ENEMIES; enemy2Index++) {
        if (isEnemy_2Active(pGame->pEnemies_2[enemy2Index]) == true) {
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
