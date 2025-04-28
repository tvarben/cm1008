#ifndef text_h
#define text_h

typedef struct text Text;

Text *createText(SDL_Renderer *pRenderer, int r, int g, int b, TTF_Font *pFont, char *pString, int x, int y);
void drawText(Text *pText);
void destroyText(Text *pText);
const SDL_Rect *getTextRect(Text *pText);
void setTextColor(Text *pText, int r, int g, int b, TTF_Font *pFont, const char *pString);

#endif