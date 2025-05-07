#include "tick.h"

bool timeToUpdate(Uint32 *pLastUpdate, Uint32 interval) {
	Uint32 now = SDL_GetTicks();
	if (now - *pLastUpdate >= interval) return true;
	else return false;
}