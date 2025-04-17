#include "../include/sound.h"
#include <stdio.h>

<<<<<<< Updated upstream
int initMusic(Mix_Music **ppMusic, char *filepath) {
	int frequency = MIX_DEFAULT_FREQUENCY;
	Uint16 format = MIX_DEFAULT_FORMAT;
	int nChannels = 2;
	int chunkSize = 4096;

	if(Mix_OpenAudio(frequency, format, nChannels, chunkSize) != 0) {
		printf("Error: %s\n", Mix_GetError());
		return 0;
	}
	*ppMusic = Mix_LoadMUS(filepath);
	if(!*ppMusic) {
		printf("Error: %s\n", Mix_GetError());
		return 0;
	}
	return 1;
=======
Mix_Music *initMusic(char *filepath) {
  int frequency = MIX_DEFAULT_FREQUENCY;
  Uint16 format = MIX_DEFAULT_FORMAT;
  int nChannels = 2;
  int chunkSize = 4096;

  if (Mix_OpenAudio(frequency, format, nChannels, chunkSize) != 0) {
    printf("Error: %s\n", Mix_GetError());
    return NULL;
  }
  Mix_Music *pMusic = Mix_LoadMUS(filepath);
  if (!pMusic) {
    printf("Error: %s\n", Mix_GetError());
    return NULL;
  }
  return pMusic;
>>>>>>> Stashed changes
}

void playMusic(Mix_Music *pMusic, int loops) {
  if (Mix_PlayingMusic() == 0) {
    if (Mix_PlayMusic(pMusic, loops) == -1) {
      printf("Error: %s\n", Mix_GetError());
    }
  }
}

<<<<<<< Updated upstream
=======
void pauseMusic() { Mix_HaltMusic(); }

>>>>>>> Stashed changes
void closeMusic(Mix_Music *pMusic) {
  if (pMusic)
    Mix_FreeMusic(pMusic);
  Mix_CloseAudio();
  Mix_Quit();
}
