#include <stdio.h>
#include "sound.h"

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
}

void playMusic(Mix_Music *pMusic, int loops) {
	if(Mix_PlayingMusic() == 0) {
		if(Mix_PlayMusic(pMusic, loops) == -1) {
			printf("Error: %s\n", Mix_GetError());
		}
	}
}

void closeMusic(Mix_Music *pMusic) {
	if(pMusic) Mix_FreeMusic(pMusic);
	Mix_CloseAudio();
	Mix_Quit();
}