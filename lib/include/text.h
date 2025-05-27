#ifndef text_h
#define text_h

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
typedef struct text Text;

Text *createText(SDL_Renderer *pRenderer, int r, int g, int b, TTF_Font *pFont, char *pString,
                 int x, int y);
void setTextColor(Text *pText, int r, int g, int b, TTF_Font *pFont, const char *pString);
void drawText(Text *pText);
const SDL_Rect *getTextRect(Text *pText);
void destroyText(Text *pText);

#endif
