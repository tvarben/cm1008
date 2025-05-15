#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include "ship.h"
#include <SDL2/SDL_mixer.h>
#include <time.h>
#include "sound.h"
#include "text.h"
#include "stars.h"
#include "enemies.h"
#include "../include/enemy2.h"
#include "../include/enemy3.h"
#include "bullet.h"
#include "cannon.h"
#include "menu.h"
#define MAX_BULLETS 1000
#define MAX_ENEMIES 100
#define MAX_ENEMIES2 25
#define WINDOW_WIDTH 1160
#define WINDOW_HEIGHT 700
#define MUSIC_FILEPATH "./resources/music.wav"
#define SHOOT_SFX_FILEPATH "./resources/pew.wav"

enum GameState { START, ONGOING, PAUSED, GAME_OVER };
typedef enum GameState GameState;

typedef struct
{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Ship *pShip;
    GameState state;
    Mix_Chunk *pSFX;
    Mix_Music *pMusic;
	TTF_Font *pFont, *pSmallFont;
	Text *pSingleplayerText, *pGameName, *pExitText, *pPauseText, *pScoreText, *pMultiplayerText, *pMenuText, *pGameOverText,
    *pMapName1, *pMapName2;
    Stars *pStars;
    EnemyImage *pEnemyImage;
    Enemy *pEnemies[MAX_ENEMIES];
    int nrOfEnemies;
    EnemyImage2 *pEnemyImage2; // new enemy2
    Enemy2 *pEnemies2[MAX_ENEMIES];
    EnemyImage3 *pEnemyImage3; // enemy3
    Enemy3 *pEnemies3[1];
    int nrOfEnemies2; 
    int nrOfEnemies3; 
    int timeForNextEnemy;
    int startTime;//in ms
    int gameTime;//in s
    Uint64 pauseStartTime;
    Uint64 pausedTime;
    Cannon *pCannon;
    Bullet *pProjectiles[MAX_BULLETS];
    SDL_Texture *pStartImage, *pStartImage2, *pMapImage1, *pMapImage2, *pMapBackground2, *pMap2Planet,
    *MAP2PLANET2, *MAP2PLANET3;
    bool networkMenu;
    bool mapMenu;
    Uint32 attackDelay;
    Uint32 lastAttackTime;

} Game;

int getTime(Game *pGame);
void updateGameTime(Game *pGame);
void resetEnemy(Game *pGame);
void resetBoss(Game *pGame);
void spawnEnemies(Game *pGame, int ammount);
void spawnBoss(Game *pGame);
void updateEnemies(Game *pGame, int *ammount);
bool areTheyAllDead(Game *pGame);

int initiate(Game *pGame) 
{
    srand(time(NULL));
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

    SDL_Surface *tempSurface3 = IMG_Load("resources/MAP1.png");
    if (!tempSurface2) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->pMapImage1 = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface3);
    SDL_FreeSurface(tempSurface3);
    if (!pGame->pMapImage1) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }
    SDL_Surface *someoneChangeTheNamesOfThesePls = IMG_Load("resources/MAP2.png");
    if (!someoneChangeTheNamesOfThesePls) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->pMapImage2 = SDL_CreateTextureFromSurface(pGame->pRenderer, someoneChangeTheNamesOfThesePls);
    SDL_FreeSurface(someoneChangeTheNamesOfThesePls);
    if (!pGame->pMapImage2) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }



    SDL_Surface *tempSurface4 = IMG_Load("resources/MAP2BACKGROUND.png");
    if (!tempSurface4) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->pMapBackground2 = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface4);
    SDL_FreeSurface(tempSurface4);
    if (!pGame->pMapBackground2) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }
    
    SDL_Surface *tempSurface5 = IMG_Load("resources/BigRed.png");
    if (!tempSurface5) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->pMap2Planet = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface5);
    SDL_FreeSurface(tempSurface5);
    if (!pGame->pMap2Planet) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }
    SDL_Surface *tempSurface6 = IMG_Load("resources/redBall.png");
    if (!tempSurface6) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->MAP2PLANET2 = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface6);
    SDL_FreeSurface(tempSurface6);
    if (!pGame->MAP2PLANET2) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return 0;
    }
    SDL_Surface *tempSurface7 = IMG_Load("resources/shattered_planet.png");
    if (!tempSurface7) {
        printf("Image Load Error: %s\n", IMG_GetError());
        return 0;
    }
    pGame->MAP2PLANET3 = SDL_CreateTextureFromSurface(pGame->pRenderer, tempSurface7);
    SDL_FreeSurface(tempSurface7);
    if (!pGame->MAP2PLANET3) {
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
    pGame->pMapName1 = createText(pGame->pRenderer,238,168,65,pGame->pSmallFont,"MAP1",WINDOW_WIDTH/3,500);
    pGame->pMapName2 = createText(pGame->pRenderer,238,168,65,pGame->pSmallFont,"MAP2",WINDOW_WIDTH-400,500);

    pGame->pEnemyImage = initiateEnemy(pGame->pRenderer);
    pGame->pEnemyImage2 = initiateEnemy2(pGame->pRenderer);
    pGame->pEnemyImage3 = initiateEnemy3(pGame->pRenderer);
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
    pGame->nrOfEnemies2 = 0;
    pGame->nrOfEnemies3 = 0;
    resetEnemy(pGame);
    pGame->timeForNextEnemy = 2;
    pGame->networkMenu = false;
    pGame->mapMenu = false;
    pGame->state = START;
    return 1;
}

void run(Game *pGame)
{
    int mapPicked = 0;
    int killedEnemies = 0;
    int nrOfEnemiesToSpawn = 4;
    bool isRunning = true;
    SDL_Rect emptyRect={0,0,0,0}, rectArray[MAX_PROJECTILES] = {0,0,0,0};
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
        const SDL_Rect *mapName1Rect = getTextRect(pGame->pMapName1);
        const SDL_Rect *mapName2Rect = getTextRect(pGame->pMapName2);


        if (SDL_PointInRect(&mousePoint, startRect))
        {
            setTextColor(pGame->pSingleplayerText, 255, 100, 100, pGame->pSmallFont, "Singleplayer");
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

        if (SDL_PointInRect(&mousePoint, mapName1Rect))
        {
            setTextColor(pGame->pMapName1, 255, 100, 100, pGame->pSmallFont, "MAP1");
        }
        else
        {
            setTextColor(pGame->pMapName1, 238, 168, 65, pGame->pSmallFont, "MAP1");
        }
        if (SDL_PointInRect(&mousePoint, mapName2Rect))
        {
            setTextColor(pGame->pMapName2, 255, 100, 100, pGame->pSmallFont, "MAP2");
        }
        else
        {
            setTextColor(pGame->pMapName2, 238, 168, 65, pGame->pSmallFont, "MAP2");
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
                    pGame->mapMenu = true;   
                } 
                else if (SDL_PointInRect(&mousePoint, exitRect))
                {
                    isRunning = false;
                }
                else if (SDL_PointInRect(&mousePoint, multiRect))
                {
                    pGame->networkMenu = true;
                }
                else if (SDL_PointInRect(&mousePoint, mapName1Rect))
                {
                    resetShip(pGame->pShip);
                    resetEnemy(pGame);
                    spawnEnemies(pGame, nrOfEnemiesToSpawn);
                    spawnBoss(pGame);
                    pGame->state = ONGOING;
                    pGame->networkMenu = false;
                    pGame->mapMenu = false;
                    pGame->pausedTime = 0;
                    pGame->startTime = SDL_GetTicks64();
                    mapPicked = 1;
                    pGame->gameTime = -1;    
                }
                else if (SDL_PointInRect(&mousePoint, mapName2Rect))
                {
                    resetShip(pGame->pShip);
                    resetEnemy(pGame);
                    nrOfEnemiesToSpawn = 20;
                    spawnEnemies(pGame, nrOfEnemiesToSpawn);
                    spawnBoss(pGame);
                    pGame->state = ONGOING;
                    pGame->networkMenu = false;
                    pGame->mapMenu = false;
                    pGame->pausedTime = 0;
                    pGame->startTime = SDL_GetTicks64();
                    mapPicked = 2;
                    pGame->gameTime = -1;    
                }
            }
            else if (pGame->state == START && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            {
                pGame->networkMenu = false;
                pGame->mapMenu= false;
            }
            else if (pGame->state == START && pGame->networkMenu == true && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE && stringIndex > 0)
            {
                ipAdress[stringIndex-1] = '\0';
                stringIndex--;
            }
            else if (pGame->state == START && pGame->networkMenu == true && event.type == SDL_KEYDOWN)
            {
                SDL_Keycode keycode = event.key.keysym.sym;
                if (((keycode >= '0' && keycode <= '9') || keycode == '.') && stringIndex < 15)
                {
                    ipAdress[stringIndex] = (char)keycode;
                    stringIndex++;
                }
            }
            
            else if (pGame->state == ONGOING && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_V)
            {
                printf("CURRENT NUMBER OF ENEMIES: %d \n",((pGame->nrOfEnemies) + (pGame->nrOfEnemies2)));
                for (int i = 0; i < pGame->nrOfEnemies; i++) {
                    if (isEnemyActive(pGame->pEnemies[i]) == false) {
                      printf("Enemy %d is inactive \n", i);
                    } else {
                      printf("Enemy %d is active \n", i);
                    }
                  }
                for (int j = 0; j < pGame->nrOfEnemies2; j++) {
                    if (isEnemy2Active(pGame->pEnemies2[j]) == false) {
                      printf("Enemy %d is inactive \n", j);
                    } else {
                      printf("Enemy %d is active \n", j);
                    }
                  }
                  
                if (isEnemy3Active(pGame->pEnemies3[0]) == false) {
                    printf("Boss is inactive \n");
                } else {
                    printf("Boss is active \n");
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
                    playSound(&pGame->pSFX,SHOOT_SFX_FILEPATH, 1);
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
                killedEnemies = 0;
                pGame->state = ONGOING;
            }
            else if (pGame->state == GAME_OVER && event.type == SDL_MOUSEBUTTONDOWN)
            {
                if(SDL_PointInRect(&mousePoint, MenuRect))
                {
                    resetShip(pGame->pShip);
                    resetEnemy(pGame);
                    resetBoss(pGame);
                    resetAllBullets();
                    resetHealth(pGame->pShip);
                    nrOfEnemiesToSpawn = 4;
                    for(int i=0;i<MAX_PROJECTILES;i++)
                    {
                      rectArray[i]=emptyRect;
                    }
                    pGame->state = START;
                    killedEnemies = 0;
                }
            }   
        }

        if (pGame->state == ONGOING) 
        {
            if (Mix_PlayingMusic()) Mix_HaltMusic();
            update_projectiles(delta_time); // update based on time since last frame passed
            updateGameTime(pGame);
            updateEnemies(pGame, &nrOfEnemiesToSpawn);
            updateShipVelocity(pGame->pShip);           // resolve velocity based on key states
            updateShip(pGame->pShip);
            updateCannon(pGame->pCannon, pGame->pShip);
            for(int i=0;i<pGame->nrOfEnemies;i++){
                 updateEnemy(pGame->pEnemies[i]);
            }
            for (int i = 0; i < pGame->nrOfEnemies2; i++) {
                updateEnemy2(pGame->pEnemies2[i]);
              }
            updateEnemy3(pGame->pEnemies3[0]);
            SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 0);
            SDL_RenderClear(pGame->pRenderer);
            if (mapPicked == 1)
            {
                SDL_SetRenderDrawColor(pGame->pRenderer, 255, 255, 255, 255); // Set color to white
                drawStars(pGame->pStars,pGame->pRenderer);
                SDL_Rect dstRect = { WINDOW_WIDTH/2.5, WINDOW_HEIGHT/3, 200, 200 };  // adjust position and size
                SDL_RenderCopy(pGame->pRenderer, pGame->pStartImage, NULL, &dstRect);
           }
           else if(mapPicked == 2)
           {
            SDL_SetRenderDrawColor(pGame->pRenderer, 255, 0, 0, 0); // Set color to white
            SDL_Rect dstRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
            SDL_RenderCopy(pGame->pRenderer, pGame->pMapBackground2, NULL, &dstRect);
            drawStars(pGame->pStars,pGame->pRenderer);
            SDL_Rect dstRect2 = { WINDOW_WIDTH/3, WINDOW_HEIGHT/3.5, 128*3, 97*3 };
            SDL_RenderCopy(pGame->pRenderer, pGame->pMap2Planet, NULL, &dstRect2);
            SDL_Rect dstRect3 = { 100, 65, 48, 48 };
            SDL_RenderCopy(pGame->pRenderer, pGame->MAP2PLANET2, NULL, &dstRect3);
           }
            drawShip(pGame->pShip);
            drawCannon(pGame->pCannon);
            render_projectiles(pGame->pRenderer); // test      
            
            if (isPlayerDead(pGame->pShip) == true)
            {
                pGame->state = GAME_OVER;
            }
            for(int i=0;i<pGame->nrOfEnemies;i++)
            {
                if (isEnemyActive(pGame->pEnemies[i]) == true)
                {
                    drawEnemy(pGame->pEnemies[i]);
                }             
            }
            for (int i = 0; i < pGame->nrOfEnemies2; i++) {
                if (isEnemy2Active(pGame->pEnemies2[i]) == true) {
                  drawEnemy2(pGame->pEnemies2[i]);
                }
              }
            if (isEnemy3Active(pGame->pEnemies3[0]) == true) {
                drawEnemy3(pGame->pEnemies3[0]);
            }
             for(int i=0;i<pGame->nrOfEnemies;i++){
                if(shipCollision(pGame->pShip, getRectEnemy(pGame->pEnemies[i]))){
                    damageEnemy(pGame->pEnemies[i], 2 , i);
                    killedEnemies++;
                    damageShip(pGame->pShip, 1);
                }
            }
            for (int j = 0; j < pGame->nrOfEnemies2; j++) {
                if (shipCollision(pGame->pShip, getRectEnemy2(pGame->pEnemies2[j]))) {
                  damageEnemy2(pGame->pEnemies2[j], 1, j);
                  damageShip(pGame->pShip, 2);
                  if(isEnemy2Active(pGame->pEnemies2[j]) == false)
                  {
                    killedEnemies++;
                  }
                }
            }
            if (shipCollision(pGame->pShip, getRectEnemy3(pGame->pEnemies3[0]))) {
                damageEnemy3(pGame->pEnemies3[0], 1, 0);
                damageShip(pGame->pShip, 2);
                if(isEnemy3Active(pGame->pEnemies3[0]) == false)
                {
                    killedEnemies++;
                }
            }
            if (pGame->gameTime >= 300)
            {
                pGame->state = GAME_OVER;
            }
            getProjectileRects(rectArray);
            for (int i = 0; i < MAX_PROJECTILES; i++)
            {
                SDL_Rect bulletRect = rectArray[i];
                for (int k = 0; k < pGame->nrOfEnemies; k++)
                {
                    SDL_Rect enemyRect = getRectEnemy(pGame->pEnemies[k]);
                    if (SDL_HasIntersection(&enemyRect, &bulletRect))
                    {
                        //printf("enemy num: %d \n", k);
                        printEnemyHealth(pGame->pEnemies[k]);
                        damageEnemy(pGame->pEnemies[k], 1, k);
                        if (isEnemyActive(pGame->pEnemies[k]) == false)
                        {
                            killedEnemies++;
                        }
                        removeProjectile(i);
                        rectArray[i]=emptyRect;
                    }
                }
                for (int j = 0; j < pGame->nrOfEnemies2; j++) {
                    SDL_Rect enemyRect2 = getRectEnemy2(pGame->pEnemies2[j]);
                    if (SDL_HasIntersection(&enemyRect2, &bulletRect)) {
                      //printf("enemy num: %d \n", j);
                      printEnemy2Health(pGame->pEnemies2[j]);
                      damageEnemy2(pGame->pEnemies2[j], 1, j);
                      if (isEnemy2Active(pGame->pEnemies2[j]) == false) {
                        killedEnemies++;
                      }
                      removeProjectile(i);
                      rectArray[j] = emptyRect;
                    }
                  }
                SDL_Rect enemyRect3 = getRectEnemy3(pGame->pEnemies3[0]);
                if (SDL_HasIntersection(&enemyRect3, &bulletRect)) {
                    //printf("enemy num: %d \n", 0);
                    printEnemy3Health(pGame->pEnemies3[0]);
                    damageEnemy3(pGame->pEnemies3[0], 1, 0);
                    if (isEnemy3Active(pGame->pEnemies3[0]) == false) {
                        killedEnemies++;
                    }
                    removeProjectile(i);
                    rectArray[i]=emptyRect;
                }
            }
            if (pGame->pScoreText) drawText(pGame->pScoreText);
            SDL_RenderPresent(pGame->pRenderer);
        } 
        else if (pGame->state == START) 
        {
            
            SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 0);      //Important to set the color before clearing the screen 
            SDL_RenderClear(pGame->pRenderer);                         //Clear the first frame when the game starts, otherwise issues on mac/linux 
            drawText(pGame->pSingleplayerText);                        //Clear the first frame when the game starts, otherwise issues on mac/linux 
            drawText(pGame->pMultiplayerText);
            SDL_SetRenderDrawColor(pGame->pRenderer, 255, 255, 255, 255); // Set color to white
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
            if (pGame->mapMenu == true)
            {
                showMapMenu(pGame->pRenderer, pGame->pSmallFont);
                drawText(pGame->pMapName1);
                drawText(pGame->pMapName2);
                SDL_Rect dstRect3 = { 220, 275, 300, 180 };  // adjust position and size
                SDL_RenderCopy(pGame->pRenderer, pGame->pMapImage1, NULL, &dstRect3);
                SDL_Rect dstRect315 = { WINDOW_WIDTH-550, 275, 300, 180 };  // adjust position and size
                SDL_RenderCopy(pGame->pRenderer, pGame->pMapImage2, NULL, &dstRect315);
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
            char scoreText[64];
            sprintf(scoreText, "You Killed %d Aliens", killedEnemies);
            Text *pKillCountText = createText(pGame->pRenderer, 238, 168, 65, pGame->pSmallFont, scoreText, WINDOW_WIDTH/2, WINDOW_HEIGHT/3);
            drawText(pKillCountText);
            destroyText(pKillCountText);
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
    for (int i = 0; i < pGame->nrOfEnemies; i++){
        destroyEnemy(pGame->pEnemies[i]);
        destroyEnemy2(pGame->pEnemies2[i]);
    }
    destroyEnemy3(pGame->pEnemies3[0]);
    if (pGame->pEnemyImage) {
        destroyEnemyImage(pGame->pEnemyImage);
        destroyEnemyImage2(pGame->pEnemyImage2);
        destroyEnemyImage3(pGame->pEnemyImage3);
    }
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
        if(pGame->pFont) pGame->pScoreText = createText(pGame->pRenderer,238,168,65,pGame->pSmallFont,scoreString,WINDOW_WIDTH/2,50);    
    }
}

void spawnEnemies(Game *pGame, int ammount)
{
    for (int i = 0; i < ammount; i++) {
        pGame->pEnemies[pGame->nrOfEnemies] = createEnemy(pGame->pEnemyImage, WINDOW_WIDTH, WINDOW_HEIGHT);
        pGame->pEnemies2[pGame->nrOfEnemies2] = createEnemy2(pGame->pEnemyImage2, WINDOW_WIDTH, WINDOW_HEIGHT);
        pGame->nrOfEnemies++;
        pGame->nrOfEnemies2++;
      }
}
void spawnBoss(Game *pGame)
{
    pGame->pEnemies3[0] = createEnemy3(pGame->pEnemyImage3, WINDOW_WIDTH, WINDOW_HEIGHT);
    pGame->nrOfEnemies3++;
}

void updateEnemies(Game *pGame, int *ammount)
{
    if (areTheyAllDead(pGame)) // yes this looks like shit
    {
        (*ammount) += 2;
        pGame->nrOfEnemies = 0;
        pGame->nrOfEnemies2 = 0;
        spawnEnemies(pGame, *ammount);
    }
}
void resetEnemy(Game *pGame) {
    for (int i = 0; i < pGame->nrOfEnemies; i++) {
      destroyEnemy(pGame->pEnemies[i]);
    }
    pGame->nrOfEnemies = 0;
    for (int j = 0; j < pGame->nrOfEnemies2; j++) {
      destroyEnemy2(pGame->pEnemies2[j]);
    }
    pGame->nrOfEnemies = 0;
    pGame->nrOfEnemies2 = 0;
    // add for new enemy here
  }
void resetBoss(Game *pGame)
{
    destroyEnemy3(pGame->pEnemies3[0]);
    pGame->nrOfEnemies3 = 0;
}

bool areTheyAllDead(Game *pGame) {
    for (int i = 0; i < pGame->nrOfEnemies; i++) {
      if (isEnemyActive(pGame->pEnemies[i]) == true || isEnemy2Active(pGame->pEnemies2[i]) == true) {
        return false;
      }
    }
    return true;
  }

int main(int argc, char** argv)
{
    Game game = {NULL, NULL, NULL, START};
    if (!initiate(&game)) return 1;

    run(&game);
    closeGame(&game);
    return 0;
}