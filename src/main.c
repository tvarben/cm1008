#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include "ship.h"
#include <SDL2/SDL_mixer.h>
#include "sound.h"
#include "text.h"
#include "stars.h"
#include "enemies.h"
#include "bullet.h"
#include "cannon.h"
#include "menu.h"
#define MAX_BULLETS 100
#define MAX_ENEMIES 30
#define WINDOW_WIDTH 1160
#define WINDOW_HEIGHT 700
#define MUSIC_FILEPATH "./resources/music.wav"

enum GameState { START, ONGOING, PAUSED, GAME_OVER };
typedef enum GameState GameState;

typedef struct
{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ship *pShip;
    GameState state;
    Mix_Music *pMusic;
	TTF_Font *pFont, *pSmallFont;
	Text *pSingleplayerText, *pGameName, *pExitText, *pPauseText, *pScoreText, *pMultiplayerText, *pMenuText, *pGameOverText;
    Stars *pStars;
    EnemyImage *pEnemyImage;
    Enemy *pEnemies[MAX_ENEMIES];
    int nrOfEnemies;
    int timeForNextEnemy;
    int startTime;//in ms
    int gameTime;//in s
    Uint64 pauseStartTime;
    Uint64 pausedTime;
    Cannon *pCannon;
    Bullet *pProjectiles[MAX_BULLETS];
    SDL_Texture *pStartImage, *pStartImage2;
    bool networkMenu;
    Uint32 attackDelay;
    Uint32 lastAttackTime;

} Game;

int getTime(Game *pGame);
void updateGameTime(Game *pGame);
void updateNrOfEnemies(Game *pGame);
void resetEnemy(Game *pGame);

int initiate(Game *pGame) 
{
    Mix_Init(MIX_INIT_WAVPACK);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0)
    {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return 0;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        printf("SDL_image Init Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 0;
    }
	if(TTF_Init()!=0)
    {
        printf("Error: %s\n",TTF_GetError());
        SDL_Quit();
        return 0;
    }

    pGame->pWindow = SDL_CreateWindow("",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!pGame->pWindow)
    {
        printf("Window Error: %s\n", SDL_GetError());
        return 0;
    }

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->pRenderer)
    {
        printf("Renderer Error: %s\n", SDL_GetError());
        return 0;
    }

	pGame->pFont = TTF_OpenFont("arial.ttf", 100);
    pGame->pSmallFont = TTF_OpenFont("arial.ttf", 50);
    if(!pGame->pFont)
    {
        printf("Error: %s\n",TTF_GetError());
        return 0;
    }

    SDL_Surface *tempSurface = IMG_Load("resources/Earth.png");
    if (!tempSurface) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->pStartImage = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    if (!pGame->pStartImage) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }
    
    SDL_Surface *tempSurface2 = IMG_Load("resources/Asteroid.png");
    if (!tempSurface2) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->pStartImage2 = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface2);
    SDL_FreeSurface(tempSurface2);
    if (!pGame->pStartImage2) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }


    pGame->pStars = createStars(WINDOW_WIDTH*WINDOW_HEIGHT/10000,WINDOW_WIDTH,WINDOW_HEIGHT);
    pGame->pGameName = createText(pGame->pRenderer,238,168,65,pGame->pFont,"SpaceShooter",WINDOW_WIDTH/2,WINDOW_HEIGHT/8);
    pGame->pSingleplayerText = createText(pGame->pRenderer,238,168,65,pGame->pSmallFont,"Singleplayer",WINDOW_WIDTH/2, 330);
    pGame->pMultiplayerText = createText(pGame->pRenderer,238,168,65,pGame->pSmallFont,"Multiplayer",WINDOW_WIDTH/2, 450);
    pGame->pExitText = createText(pGame->pRenderer,238,168,65,pGame->pSmallFont,"Exit",WINDOW_WIDTH/2, 570);
    pGame->pPauseText = createText(pGame->pRenderer,238,168,65,pGame->pFont,"PAUSED",WINDOW_WIDTH/2,WINDOW_HEIGHT/4);
    pGame->pMenuText = createText(pGame->pRenderer,238,168,65,pGame->pFont,"MENU",WINDOW_WIDTH/2,450);
    pGame->pGameOverText = createText(pGame->pRenderer,238,168,65,pGame->pFont,"GAME OVER",WINDOW_WIDTH/2,WINDOW_HEIGHT/5);
    pGame->pEnemyImage = initiateEnemy(pGame->pRenderer);
    pGame->pCannon = createCannon(pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    pGame->attackDelay = 250;
    pGame->lastAttackTime = 0;

    pGame->pShip = createShip(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!pGame->pShip)
    {
        printf("Ship creation failed.\n");
        return 0;
    }

    if (!initMusic(&pGame->pMusic, MUSIC_FILEPATH))
    {
        printf("Error: %s\n",Mix_GetError());
        return 0;
    }

    pGame->nrOfEnemies = 0;
    resetEnemy(pGame);
    pGame->timeForNextEnemy = 2;
    pGame->networkMenu = false;
    pGame->state = START;
    return 1;
}

void run(Game *pGame)
{
    bool isRunning = true;
    SDL_Rect rectArray[MAX_PROJECTILES] = {0,0,0,0};
    char ipAdress[16] = {""};
    int stringIndex = 0;
    SDL_Event event;
    playMusic(pGame->pMusic, -1);

    Uint32 last_time = SDL_GetTicks(); // timer for rendering bullets
    render_projectiles(pGame->pRenderer);

    while (isRunning)
    {
        int x, y;
        Uint32 current_time = SDL_GetTicks(); // needed for shooting
        float delta_time = (current_time - last_time) /
        1000.0f; // calculates time passed since last frame
        last_time = current_time;   // needed for shooting
        Uint32 now = SDL_GetTicks();
        SDL_GetMouseState(&x,&y);
        SDL_Point mousePoint = {x,y};       //Kolla position för musen

        const SDL_Rect *startRect = getTextRect(pGame->pSingleplayerText);     //Hämta position för rect för Start-texten
        const SDL_Rect *exitRect = getTextRect(pGame->pExitText);       //Hämta position för rect för Exit-texten
        const SDL_Rect *multiRect = getTextRect(pGame->pMultiplayerText);
        const SDL_Rect *MenuRect = getTextRect(pGame->pMenuText);


        if (SDL_PointInRect(&mousePoint, startRect))
        {
            setTextColor(pGame->pSingleplayerText, 255, 255, 100, pGame->pSmallFont, "Singleplayer");
        }
        else
        {
            setTextColor(pGame->pSingleplayerText, 238, 168, 65, pGame->pSmallFont, "Singleplayer");
        }
        if (SDL_PointInRect(&mousePoint, exitRect))
        {
            setTextColor(pGame->pExitText, 255, 100, 100, pGame->pSmallFont, "Exit");
        } 
        else
        {
            setTextColor(pGame->pExitText, 238, 168, 65, pGame->pSmallFont, "Exit");
        }
        if (SDL_PointInRect(&mousePoint, multiRect))
        {
            setTextColor(pGame->pMultiplayerText, 255, 100, 100, pGame->pSmallFont, "Multiplayer");
        }
        else
        {
            setTextColor(pGame->pMultiplayerText, 238, 168, 65, pGame->pSmallFont, "Multiplayer");
        }

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
            else if (pGame->state == START && event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (SDL_PointInRect(&mousePoint, startRect))
                {
                    resetShip(pGame->pShip);
                    resetEnemy(pGame);
                    pGame->state = ONGOING;
                    pGame->networkMenu = false;
                    pGame->pausedTime = 0;
                    pGame->startTime = SDL_GetTicks64();
                    pGame->gameTime = -1;       
                } 
                else if (SDL_PointInRect(&mousePoint, exitRect))
                {
                    isRunning = false;
                }
                else if (SDL_PointInRect(&mousePoint, multiRect))
                {
                    pGame->networkMenu = true;
                }
            }
            else if (pGame->state == START && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            {
                pGame->networkMenu = false;
            }
            else if (pGame->state == START && pGame->networkMenu == true && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE && stringIndex > 0)
            {
                ipAdress[stringIndex-1] = '\0';
                stringIndex--;
            }
            else if (pGame->state == START && pGame->networkMenu == true && event.type == SDL_KEYDOWN)
            {
                SDL_Keycode keycode = event.key.keysym.sym;
                if ((char)keycode <= '9' && (char)keycode >= '0' && stringIndex < 15 || (char)keycode =='.' )
                {
                    ipAdress[stringIndex] = (char)keycode;
                    stringIndex++;
                }
            }
            else if (pGame->state == ONGOING && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            {
                pGame->state = PAUSED;
                pGame->pauseStartTime = SDL_GetTicks64();
            }   
            else if (pGame->state == ONGOING && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE)
            {
                if (now - pGame->lastAttackTime >= pGame->attackDelay)
                {
                    handleCannonEvent(pGame->pCannon, &event);
                    pGame->lastAttackTime = now;
                }
            }
            else if (pGame->state == ONGOING && (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP))
            {
                handleShipEvent(pGame->pShip, &event);  // track which keys are pressed
            }    
            else if (pGame->state == PAUSED && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            {
                pGame->pausedTime += SDL_GetTicks64() - pGame->pauseStartTime;
                pGame->state = ONGOING;
            }
            else if (pGame->state == GAME_OVER && event.type == SDL_MOUSEBUTTONDOWN)
            {
                if(SDL_PointInRect(&mousePoint, MenuRect))
                {
                    resetShip(pGame->pShip);
                    resetEnemy(pGame);
                    pGame->state = START;
                }
            }   
        }

        if (pGame->state == ONGOING) 
        {
            if (Mix_PlayingMusic()) Mix_HaltMusic();
            
            update_projectiles(delta_time); // update based on time since last frame passed
            updateGameTime(pGame);
            updateNrOfEnemies(pGame);
            updateShipVelocity(pGame->pShip);           // resolve velocity based on key states
            updateShip(pGame->pShip);
            updateCannon(pGame->pCannon, pGame->pShip);
            for(int i=0;i<pGame->nrOfEnemies;i++) updateEnemy(pGame->pEnemies[i]);
            SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 0);
            SDL_RenderClear(pGame->pRenderer);
            SDL_Rect dstRect = { WINDOW_WIDTH/2.5, WINDOW_HEIGHT/3, 200, 200 };  // adjust position and size
            SDL_RenderCopy(pGame->pRenderer, pGame->pStartImage, NULL, &dstRect);
            drawStars(pGame->pStars,pGame->pRenderer);
            drawShip(pGame->pShip);
            drawCannon(pGame->pCannon);
            render_projectiles(pGame->pRenderer); // test      
            for(int i=0;i<pGame->nrOfEnemies;i++) drawEnemy(pGame->pEnemies[i]);
            for(int i=0;i<pGame->nrOfEnemies;i++){
                if(shipCollision(pGame->pShip, getRectEnemy(pGame->pEnemies[i]))){
                    pGame->state = GAME_OVER;
                }
            }
            if(pGame->pScoreText) drawText(pGame->pScoreText);
            getProjectileRects(rectArray);
            for (int i = 0; i < MAX_PROJECTILES; i++)
            {
                SDL_Rect bulletRect = rectArray[i];
                for (int k = 0; k < pGame->nrOfEnemies; k++)
                {
                    SDL_Rect enemyRect = getRectEnemy(pGame->pEnemies[k]);
                    if (SDL_HasIntersection(&enemyRect, &bulletRect))
                    {
                        printf("CLOWN DOWN \n");
                        disableEnemy(pGame->pEnemies[k]);
                    }
                }
            }
            if (pGame->pScoreText)
            drawText(pGame->pScoreText);
            SDL_RenderPresent(pGame->pRenderer);
        } 
        else if (pGame->state == START) 
        {
            
            SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 0);  //Important to set the color before clearing the screen 
            SDL_RenderClear(pGame->pRenderer);                         //Clear the first frame when the game starts, otherwise issues on mac/linux 
            drawText(pGame->pSingleplayerText);                        //Clear the first frame when the game starts, otherwise issues on mac/linux 
            drawText(pGame->pMultiplayerText);
            drawStars(pGame->pStars,pGame->pRenderer);
            drawText(pGame->pExitText);
            drawText(pGame->pGameName);
            SDL_Rect dstRect = { 125, 500, 100, 100 };  // adjust position and size
            SDL_RenderCopy(pGame->pRenderer, pGame->pStartImage, NULL, &dstRect);
            SDL_Rect dstRect2 = { 1000, 125, 50, 50 };  // adjust position and size
            SDL_RenderCopy(pGame->pRenderer, pGame->pStartImage2, NULL, &dstRect2);
            if (pGame->networkMenu == true)
            {
                showNetworkMenu(pGame->pRenderer, pGame->pSmallFont, ipAdress);
            }
            SDL_RenderPresent(pGame->pRenderer);    //Draw the start text
        }
        else if (pGame->state == PAUSED)
        {
            drawText(pGame->pPauseText);
            SDL_RenderPresent(pGame->pRenderer);
        }   
        else if (pGame->state == GAME_OVER)
        {
            SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 0);  
            drawText(pGame->pGameOverText);
            drawText(pGame->pMenuText);
            SDL_RenderPresent(pGame->pRenderer);
            if (SDL_PointInRect(&mousePoint, MenuRect)) {
                setTextColor(pGame->pMenuText, 255, 255, 100, pGame->pFont, "MENU");
            }
            else {
                setTextColor(pGame->pMenuText, 238, 168, 65, pGame->pFont, "MENU");
            }
        }   
    }
}

void closeGame(Game *pGame)
{
    if (pGame->pShip) destroyShip(pGame->pShip);
    if (pGame->pShip) destroyCannon(pGame->pCannon);
    if(pGame->pStars) destroyStars(pGame->pStars);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    if(pGame->pSingleplayerText) destroyText(pGame->pSingleplayerText);
    if(pGame->pMultiplayerText) destroyText(pGame->pMultiplayerText);
    if(pGame->pExitText) destroyText(pGame->pExitText);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont); 
    for(int i=0;i<pGame->nrOfEnemies;i++) destroyEnemy(pGame->pEnemies[i]);
    if(pGame->pEnemyImage) destroyEnemyImage(pGame->pEnemyImage);
    closeMusic(pGame->pMusic);
    IMG_Quit();
    SDL_Quit();
}


int getTime(Game *pGame)
{
    return (SDL_GetTicks64() - pGame->startTime - pGame->pausedTime) / 1000;
}

void updateGameTime(Game *pGame)
{
    if(getTime(pGame)>pGame->gameTime && pGame->state == ONGOING)
    {
        (pGame->gameTime)++;
        if(pGame->pScoreText) destroyText(pGame->pScoreText);
        static char scoreString[30];
        sprintf(scoreString,"%d",getTime(pGame));
        if(pGame->pFont) pGame->pScoreText = createText(pGame->pRenderer,238,168,65,pGame->pFont,scoreString,WINDOW_WIDTH-50,50);    
    }
}

void updateNrOfEnemies(Game *pGame)
{
    if(getTime(pGame)>pGame->timeForNextEnemy && pGame->nrOfEnemies<MAX_ENEMIES)
    {
        (pGame->timeForNextEnemy)+=1;//seconds till next enemy
        pGame->pEnemies[pGame->nrOfEnemies] = createEnemy(pGame->pEnemyImage,WINDOW_WIDTH,WINDOW_HEIGHT);
        pGame->nrOfEnemies++; 
    }    
}

void resetEnemy(Game *pGame)
{
    for(int i=0;i<pGame->nrOfEnemies;i++) destroyEnemy(pGame->pEnemies[i]);
    pGame->nrOfEnemies = 3;
    for(int i=0;i<pGame->nrOfEnemies;i++)
    {
        pGame->pEnemies[i] = createEnemy(pGame->pEnemyImage,WINDOW_WIDTH,WINDOW_HEIGHT);
    }
}

int main(int argc, char** argv)
{
    Game game = {NULL, NULL, NULL, START};
    if (!initiate(&game)) return 1;

    run(&game);
    closeGame(&game);
    return 0;
}
