#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>

#define WINDOW_WIDTH 1160
#define WINDOW_HEIGHT 700

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0 || SDLNet_Init() != 0) {
        printf("Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    UDPsocket socket = SDLNet_UDP_Open(0); // let OS pick port
    IPaddress serverIP;
    SDLNet_ResolveHost(&serverIP, "127.0.0.1", 2000);

    UDPpacket* packet = SDLNet_AllocPacket(512);
    packet->address = serverIP;

    // Create ships (position will be corrected once server responds)
    Ship* localShip = createShip(0, 0, renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    Ship* remoteShip = createShip(0, 0, renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    int playerID = -1;
    GameState gameState = START;
    bool running = true;
    bool serverInitialized = false;
    SDL_Event event;

    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        // Send input
        ClientData clientData = { .playerNumber = playerID, .command = CMD_NONE };
        const Uint8* keys = SDL_GetKeyboardState(NULL);

        if (keys[SDL_SCANCODE_W]) clientData.command = CMD_UP;
        else if (keys[SDL_SCANCODE_S]) clientData.command = CMD_DOWN;
        else if (keys[SDL_SCANCODE_A]) clientData.command = CMD_LEFT;
        else if (keys[SDL_SCANCODE_D]) clientData.command = CMD_RIGHT;

        memcpy(packet->data, &clientData, sizeof(ClientData));
        packet->len = sizeof(ClientData);
        SDLNet_UDP_Send(socket, -1, packet);

        // Try to receive data
        if (SDLNet_UDP_Recv(socket, packet)) {
            ServerData serverData;
            memcpy(&serverData, packet->data, sizeof(ServerData));
            gameState = serverData.gState;
            playerID = serverData.playerNr;

            // Initialize positions if first packet
            if (!serverInitialized) {
                setShipPosition(localShip, serverData.ships[playerID].x, serverData.ships[playerID].y);
                setShipPosition(remoteShip, serverData.ships[!playerID].x, serverData.ships[!playerID].y);
                serverInitialized = true;
            }

            // Update remote ship
            setShipVelocity(remoteShip, serverData.ships[!playerID].vx, serverData.ships[!playerID].vy);
            setShipPosition(remoteShip, serverData.ships[!playerID].x, serverData.ships[!playerID].y);
        }

        // Predict local ship motion to counter server-side delays and feel immediate on client side)
        if (keys[SDL_SCANCODE_W]) setShipVelocity(localShip, 0, -1);
        else if (keys[SDL_SCANCODE_S]) setShipVelocity(localShip, 0, 1);
        else if (keys[SDL_SCANCODE_A]) setShipVelocity(localShip, -1, 0);
        else if (keys[SDL_SCANCODE_D]) setShipVelocity(localShip, 1, 0);
        else setShipVelocity(localShip, 0, 0);

        if (serverInitialized) {
            updateShip(localShip);
            updateShip(remoteShip);
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        if (serverInitialized) {
            drawShip(localShip);
            drawShip(remoteShip);
        }
        SDL_RenderPresent(renderer);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 8) SDL_Delay(8 - frameTime); // ~120 FPS
    }

    destroyShip(localShip);
    destroyShip(remoteShip);
    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(socket);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDLNet_Quit();
    SDL_Quit();
    return 0;
}
