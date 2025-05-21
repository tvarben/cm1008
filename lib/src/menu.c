#include "../include/ship.h"
#include "../include/ship_data.h"
#include "../include/text.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

char *enterIPAddress(SDL_Renderer *renderer, TTF_Font *font) {
    static char ip[32] = "";
    SDL_StartTextInput();
    SDL_Event event;
    bool done = false;

    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return NULL;
            if (event.type == SDL_TEXTINPUT) {
                if (strlen(ip) < 31) strcat(ip, event.text.text);
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(ip) > 0)
                    ip[strlen(ip) - 1] = '\0';
                if (event.key.keysym.sym == SDLK_RETURN) done = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_Rect box = {200, 300, 760, 80};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &box);
        SDL_SetRenderDrawColor(renderer, 238, 168, 65, 255);
        SDL_RenderDrawRect(renderer, &box);

        SDL_Color color = {255, 255, 255};
        SDL_Surface *surf = TTF_RenderText_Solid(font, ip, color);
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_Rect rect = {box.x + 10, box.y + 25, surf->w, surf->h};
        SDL_RenderCopy(renderer, tex, NULL, &rect);
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    return ip;
}
