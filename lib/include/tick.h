#ifndef TICK_H
#define TICK_H

#include <SDL2/SDL.h>
#include <stdbool.h>

bool timeToUpdate(Uint32 *pLastTick, Uint32 interval);

#endif