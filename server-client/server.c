#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include "../include/ship.h"
#include "ship_data.h"

#define WINDOW_WIDTH 1160
#define WINDOW_HEIGHT 700

// Reset ships to intended starting positions
void resetAllShips(Ship* ships[]) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        setShipPosition(ships[i], 200 + i * 400, WINDOW_HEIGHT / 2);
        setShipVelocity(ships[i], 0, 0);
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0 || SDLNet_Init() != 0) {
        printf("Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    UDPsocket socket = SDLNet_UDP_Open(2000);
    UDPpacket* packet = SDLNet_AllocPacket(512);

    Ship* ships[MAX_PLAYERS];
    IPaddress clients[MAX_PLAYERS] = {0};
    bool clientConnected[MAX_PLAYERS] = { false, false };
    int clientCount = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        // Temporary initial spawn, it will be reset later when all players are connected
        ships[i] = createShip(0, 0, renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    GameState gameState = RUNNING;
    bool running = true;
    Uint32 lastPrint = 0;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        while (SDLNet_UDP_Recv(socket, packet)) {
            int clientID = -1;
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (clientConnected[i] &&
                    packet->address.host == clients[i].host &&
                    packet->address.port == clients[i].port) {
                    clientID = i;
                    break;
                }
            }

            // New client connects
            if (clientID == -1 && clientCount < MAX_PLAYERS) {
                clientID = clientCount;
                clients[clientID] = packet->address;
                clientConnected[clientID] = true;
                clientCount++;
                printf("New client connected: %d\n", clientID);

                if (clientCount == MAX_PLAYERS) {
                    printf("All players connected. Resetting ships...\n");
                    resetAllShips(ships);
                    gameState = RUNNING;
                }
            }

            // Process input
            if (clientID >= 0 && clientID < MAX_PLAYERS) {
                ClientData cData;
                memcpy(&cData, packet->data, sizeof(ClientData));

                switch (cData.command) {
                    case CMD_UP:    setShipVelocity(ships[clientID], 0, -1); break;
                    case CMD_DOWN:  setShipVelocity(ships[clientID], 0, 1); break;
                    case CMD_LEFT:  setShipVelocity(ships[clientID], -1, 0); break;
                    case CMD_RIGHT: setShipVelocity(ships[clientID], 1, 0); break;
                    case CMD_NONE:  setShipVelocity(ships[clientID], 0, 0); break;
                }

                printf("Received input from player %d at tick %u\n", clientID, SDL_GetTicks());
            }
        }

        // Always update movement
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (clientConnected[i]) {
                updateShip(ships[i]);
            }
        }

        // Send updated data
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (clientConnected[i]) {
                ServerData sData;
                sData.gState = gameState;
                sData.playerNr = i;

                for (int j = 0; j < MAX_PLAYERS; j++) {
                    sData.ships[j].x = getShipX(ships[j]);
                    sData.ships[j].y = getShipY(ships[j]);
                    sData.ships[j].vx = getShipVx(ships[j]);
                    sData.ships[j].vy = getShipVy(ships[j]);
                }

                memcpy(packet->data, &sData, sizeof(ServerData));
                packet->len = sizeof(ServerData);
                packet->address = clients[i];
                SDLNet_UDP_Send(socket, -1, packet);
            }
        }

        // Draw server window
        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
        SDL_RenderClear(renderer);
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (clientConnected[i]) {
                drawShip(ships[i]);
            }
        }
        SDL_RenderPresent(renderer);

        // Tick log
        Uint32 now = SDL_GetTicks();
        if (now - lastPrint >= 1000) {
            printf("Server tick @ %u\n", now);
            lastPrint = now;
        }

        SDL_Delay(8); // ~120 FPS
    }

    // Clean up
    for (int i = 0; i < MAX_PLAYERS; i++) {
        destroyShip(ships[i]);
    }
    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(socket);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDLNet_Quit();
    SDL_Quit();
    return 0;
}
