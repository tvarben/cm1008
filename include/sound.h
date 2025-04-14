#ifndef SOUND_H
#define SOUND_H

#include <SDL2/SDL_mixer.h>

Mix_Music* initMusic(char *filepath);
void playMusic(Mix_Music *pMusic, int loops);
void pauseMusic();
void closeMusic(Mix_Music *pMusic);

#endif 