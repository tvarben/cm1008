#ifndef SOUND_H
#define SOUND_H

#include <SDL2/SDL_mixer.h>

int initMusic(Mix_Music **ppMusic, char *filepath);
void playMusic(Mix_Music *pMusic, int loops);
void closeMusic(Mix_Music *pMusic);
void playSound(Mix_Chunk **ppSound, char *filepath, int channel);

#endif 