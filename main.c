/*******************************************************************************************
 *
 *   Scope & Goals:
 *   This is a C port of the Dino Game from the Google Chrome browser.
 *   (https://www.google.com/chrome/terms/)
 *   This game is a work in progress and is not yet complete.
 *   The goal of this project is to learn C.
 *
 *   Dependencies:
 *   This game uses raylib. raylib is licensed wit the zlib/libpng license.
 *   (www.raylib.com)
 *
 *   Copyright & License:
 *   This code is licensed under the ISC License.
 *   (https://www.github.com/richardstephens-dev/chrome-dino-game-c-clone)
 *   Copyright (c) 2023 Richard J Stephens
 *
 ********************************************************************************************/

// Includes
//----------------------------------------------------------------------------------
#include "raylib.h"
#include <stdlib.h>
#include <math.h>
//----------------------------------------------------------------------------------

// Local Variables Definition
//----------------------------------------------------------------------------------
#define MAX_ENTITIES 99
const int MAX_FRAME_SPEED = 99;
const int MIN_FRAME_SPEED = 60;
const int FPS = 60;
const int MAX_CLOUDS = 6;
const int MAX_OBSTACLE_LENGTH = 3;
const int MAX_OBSTACLE_DUPLICATION = 2;
const int MAX_SPEED = 13;
const int SPEED = 6;
const int HEIGHT = 600;
const int WIDTH = 1280;
const int TREX_SPRITES_WIDTH = 88;
const int TREX_SPRITES_HEIGHT = 94;
const int TREX_SPRITES_WIDTH_DUCK = 236 / 2;
const int TREX_SPRITES_HEIGHT_DUCK = 60;
const int CACTUS_LARGE_SPRITE_X = 652;
const int CACTUS_LARGE_SPRITE_Y = 2;
const int CACTUS_LARGE_SPRITE_WIDTH = 25;
const int CACTUS_LARGE_SPRITE_HEIGHT = 70;
const int CACTUS_SMALL_SPRITE_X = 446;
const int CACTUS_SMALL_SPRITE_Y = 2;
const int CACTUS_SMALL_SPRITE_WIDTH = 17;
const int CACTUS_SMALL_SPRITE_HEIGHT = 35;
const int CLOUD_SPRITE_X = 166;
const int CLOUD_SPRITE_Y = 2;
const int CLOUD_SPRITE_WIDTH = 46;
const int CLOUD_SPRITE_HEIGHT = 27;
const int HORIZON_SPRITE_X = 2;
const int HORIZON_SPRITE_Y = 104;
const int HORIZON_SPRITE_WIDTH = 600;
const int PTERODACTYL_SPRITE_X = 260;
const int PTERODACTYL_SPRITE_Y = 2;
const int PTERODACTYL_SPRITE_WIDTH = 46;
const int PTERODACTYL_SPRITE_HEIGHT = 40;
const float FLOOR_Y_POS = 0.8f * HEIGHT;
const float JUMP_Y_POS = 100.0f;
const float INITIAL_JUMP_VELOCITY = -24.0f;
const float DROP_VELOCITY = 12.0f;
const float DINO_START_X_POS = 250.0f;
const float DINO_PLAY_X_POS = WIDTH / 2 + TREX_SPRITES_WIDTH;
const int MAX_OBSTACLES = 2;

enum ComponentsEnum
{
    CLOUD = 0b00000001,
    POSITION = 0b00000010,
    VELOCITY = 0b00000100,
    SPRITE = 0b00001000,
    ANIMATION = 0b00010000,
    DINO = 0b00100000,
    COLLISION = 0b01000000,
    OBSTACLE = 0b10000000,
};

enum GameState
{
    MENU,
    PLAYING,
    GAMEOVER
};

enum ObstacleType
{
    CACTUS_LARGE,
    CACTUS_SMALL,
    PTERODACTYL
};

// non const/macro variables
int nextEntityId = 0;
//----------------------------------------------------------------------------------

// Entity Component System
// ----------------------------------------------------------------------------------
typedef struct PositionComponent
{
    float x, y;
} PositionComponent;
PositionComponent positionComponents[MAX_ENTITIES];

typedef struct VelocityComponent
{
    float x, y;
} VelocityComponent;
VelocityComponent velocityComponents[MAX_ENTITIES];

typedef struct SpriteComponent
{
    Texture2D texture;
    Rectangle sourceRec;
} SpriteComponent;
SpriteComponent spriteComponents[MAX_ENTITIES];

typedef struct AnimationComponent
{
    int currentFrameIndex;
    int frameIndexSlice[2];
    int framesSpeed;
    int framesCounter;
} AnimationComponent;
AnimationComponent animationComponents[MAX_ENTITIES];

typedef struct DinoComponent
{
    bool isDucking;
    bool isJumping;
    bool isDead;
    int jumpFrameCount;
    int slideFrameCount;
} DinoComponent;
DinoComponent dinoComponents[MAX_ENTITIES];

typedef struct CollisionComponent
{
    Rectangle collisionRec;
} CollisionComponent;
CollisionComponent collisionComponents[MAX_ENTITIES];

typedef struct ObstacleComponent
{
    int type;
    int xIndex;
} ObstacleComponent;
ObstacleComponent obstacleComponents[MAX_ENTITIES];

typedef struct CloudComponent
{
    int xIndex, yIndex;
} CloudComponent;
CloudComponent cloudComponents[MAX_ENTITIES];

typedef struct Entity
{
    int id;
    int componentMask;
} Entity;

typedef struct CollisionMask
{
    int width, height;
    bool *pixels;
} CollisionMask;

//----------------------------------------------------------------------------------

// Local Functions Declaration
//----------------------------------------------------------------------------------
Entity
CreateEntity();
bool HasComponent(Entity *entities, int id, int component);
void AddComponent(Entity *entities, int id, int component);
void RemoveComponent(Entity *entities, int id, int component);

void UpdateDinoAnimationSystem(Entity *entities, Texture2D dinoTexture, Texture2D dinoDuckTexture);
void UpdateDinoPoseSystem(Entity *entities);
void UpdatePositionSystem(Entity *entities, float scrollIndex);
void UpdateVelocitySystem(Entity *entities, float scrollMultiplier);
void DrawSpriteSystem(Entity *entities);
void UpdateDinoVelocity(int i, float scrollMultiplier);
void UpdateCloudVelocity(int i, float scrollMultiplier);
void UpdateObstacleVelocity(int i, float scrollMultiplier);
void UpdateDinoPosition(int i);
void UpdateCloudPosition(int i, float scrollIndex);
void UpdateObstaclePosition(int i, float scrollIndex);
void UpdateFrameCounterSystem(Entity *entities);
void UpdateCurrentFrameIndexSystem(Entity *entities);
void UpdateObstacleTypeSystem(Entity *entities);
void UpdateCollisionSystem(Entity *entities);
void UpdateObstacleTextureSystem(Entity *entities, Texture2D cactusLargeTexture, Texture2D cactusSmallTexture, Texture2D pterodactylTexture);
bool IsCollisionMaskOverlap(Entity *entities, int i, int j);
CollisionMask GetCollisionMaskFromSprite(Entity *entities, int i);
int LoadHighScore();
void SaveHighScore(int score);
void DrawScore(int score, int highScore, Texture2D scoreTexture);

bool IsJumping(float y);
bool IsDucking(int posY);
bool IsSpriteOverlap(Rectangle rec1, Rectangle rec2);
bool IsOutOfBounds(int i);
//----------------------------------------------------------------------------------

// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(WIDTH, HEIGHT, "Dino Game");
    int gameState = MENU;

    Image dinoImage = LoadImage("resources/dino.png");
    Texture2D dinoTexture = LoadTextureFromImage(dinoImage);
    UnloadImage(dinoImage);

    Image dinoDuckImage = LoadImage("resources/dino_duck.png");
    Texture2D dinoDuckTexture = LoadTextureFromImage(dinoDuckImage);
    UnloadImage(dinoDuckImage);

    Image horizonImage = LoadImage("resources/horizon.png");
    Texture2D horizonTexture = LoadTextureFromImage(horizonImage);
    UnloadImage(horizonImage);

    Image pterodactylImage = LoadImage("resources/pterodactyl.png");
    Texture2D pterodactylTexture = LoadTextureFromImage(pterodactylImage);
    UnloadImage(pterodactylImage);

    Image restartImage = LoadImage("resources/restart.png");
    Texture2D restartTexture = LoadTextureFromImage(restartImage);
    UnloadImage(restartImage);

    Image cactusLargeImage = LoadImage("resources/cactus_large.png");
    Texture2D cactusLargeTexture = LoadTextureFromImage(cactusLargeImage);
    UnloadImage(cactusLargeImage);

    Image cactusSmallImage = LoadImage("resources/cactus_small.png");
    Texture2D cactusSmallTexture = LoadTextureFromImage(cactusSmallImage);
    UnloadImage(cactusSmallImage);

    Image cloudImage = LoadImage("resources/cloud.png");
    Texture2D cloudTexture = LoadTextureFromImage(cloudImage);
    UnloadImage(cloudImage);

    Image gameOverImage = LoadImage("resources/gameover.png");
    Texture2D gameOverTexture = LoadTextureFromImage(gameOverImage);
    UnloadImage(gameOverImage);

    Image scoreImage = LoadImage("resources/scores.png");
    Texture2D scoreTexture = LoadTextureFromImage(scoreImage);
    UnloadImage(scoreImage);

    Entity entities[MAX_ENTITIES];
    int dinoId = nextEntityId;
    entities[dinoId] = CreateEntity();
    AddComponent(entities, dinoId, POSITION);
    AddComponent(entities, dinoId, VELOCITY);
    AddComponent(entities, dinoId, SPRITE);
    AddComponent(entities, dinoId, ANIMATION);
    AddComponent(entities, dinoId, DINO);
    AddComponent(entities, dinoId, COLLISION);
    positionComponents[dinoId] = (PositionComponent){DINO_START_X_POS, FLOOR_Y_POS};
    velocityComponents[dinoId] = (VelocityComponent){0.0f, 0.0f};
    spriteComponents[dinoId] = (SpriteComponent){dinoTexture, {0.0f, 0.0f, (float)dinoTexture.width / 6, (float)dinoTexture.height}};
    animationComponents[dinoId] = (AnimationComponent){0, {2, 3}, 8, 0};
    dinoComponents[dinoId] = (DinoComponent){false, false, false, 0, 0};
    collisionComponents[dinoId] = (CollisionComponent){(Rectangle){positionComponents[dinoId].x, positionComponents[dinoId].y, (float)dinoTexture.width / 6, (float)dinoTexture.height}};

    int frameCounter = 0;
    int score = 0;
    float scrollMultiplier = 1.75;
    float scrollIndex = 0;
    int highScore = LoadHighScore();

    SetTargetFPS(60);

    for (int i = 0; i < MAX_OBSTACLES * 2; i++)
    {
        int obstacleId = nextEntityId;
        entities[obstacleId] = CreateEntity();
        AddComponent(entities, obstacleId, POSITION);
        AddComponent(entities, obstacleId, VELOCITY);
        AddComponent(entities, obstacleId, SPRITE);
        AddComponent(entities, obstacleId, OBSTACLE);
        AddComponent(entities, obstacleId, COLLISION);
        obstacleComponents[obstacleId].xIndex = i;
        positionComponents[obstacleId].x = -1000;
        UpdateObstacleTypeSystem(entities);
        UpdateObstacleTextureSystem(entities, cactusLargeTexture, cactusSmallTexture, pterodactylTexture);
        UpdateObstacleVelocity(obstacleId, 1.0f);
        UpdateObstaclePosition(obstacleId, scrollIndex);
    }

    for (int i = 0; i < MAX_CLOUDS; i++)
    {
        int cloudId = nextEntityId;
        entities[cloudId] = CreateEntity();
        AddComponent(entities, cloudId, POSITION);
        AddComponent(entities, cloudId, VELOCITY);
        AddComponent(entities, cloudId, SPRITE);
        AddComponent(entities, cloudId, CLOUD);
        velocityComponents[cloudId].x = -1.0f;
        velocityComponents[cloudId].y = 0;
        spriteComponents[cloudId].texture = cloudTexture;
        spriteComponents[cloudId].sourceRec = (Rectangle){0, 0, (float)cloudTexture.width, (float)cloudTexture.height};
        cloudComponents[cloudId].xIndex = i;
        cloudComponents[cloudId].yIndex = i;
        positionComponents[cloudId].x = i * (cloudTexture.width + 20) + GetRandomValue(0, MAX_CLOUDS / 2) * WIDTH;
        positionComponents[cloudId].y = 30 + i * (cloudTexture.height + 20);
    }
    //--------------------------------------------------------------------------------------

    // Main game loop
    //--------------------------------------------------------------------------------------
    while (!WindowShouldClose())
    {
        if (gameState == MENU)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                gameState = PLAYING;
            }
        }

        if (gameState == PLAYING)
        {
            // Update Systems
            //----------------------------------------------------------------------------------
            UpdatePositionSystem(entities, scrollIndex);
            UpdateDinoPoseSystem(entities);
            UpdateDinoAnimationSystem(entities, dinoTexture, dinoDuckTexture);
            UpdateVelocitySystem(entities, scrollMultiplier);
            UpdateObstacleTypeSystem(entities);
            UpdateFrameCounterSystem(entities);
            UpdateCurrentFrameIndexSystem(entities);
            UpdateObstacleTextureSystem(entities, cactusLargeTexture, cactusSmallTexture, pterodactylTexture);
            UpdateCollisionSystem(entities);
            //----------------------------------------------------------------------------------

            // Update game variables
            //----------------------------------------------------------------------------------
            frameCounter++;
            scrollIndex -= 2.5f * scrollMultiplier;
            scrollMultiplier *= 1.00015f;
            if (score % 100 == 0 && score != 0)
            {
                scrollMultiplier += 0.005f * score / 100;
            }

            if (scrollIndex <= -horizonTexture.width)
            {
                scrollIndex = 0;
            }
            if (frameCounter % 10 == 0)
            {
                score += fmax(1 * scrollMultiplier, 1.0f);
            }

            if (dinoComponents[dinoId].isDead)
            {
                gameState = GAMEOVER;
            }

            //----------------------------------------------------------------------------------
        }

        if (gameState == GAMEOVER)
        {
            SaveFileData("highscore.txt", &score, sizeof(int));
            DrawTexture(gameOverTexture, (WIDTH - gameOverTexture.width) / 2, (HEIGHT - gameOverTexture.height) / 2, WHITE);
            DrawTexture(restartTexture, (WIDTH - restartTexture.width) / 2, (HEIGHT - restartTexture.height) / 2 + 100, WHITE);
            UpdateDinoAnimationSystem(entities, dinoTexture, dinoDuckTexture);
            spriteComponents[dinoId].sourceRec.x = (float)spriteComponents[dinoId].sourceRec.width * (float)(animationComponents[dinoId].currentFrameIndex + animationComponents[dinoId].frameIndexSlice[0]);

            if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                                            (GetMousePosition().x >= (WIDTH - restartTexture.width) / 2 && GetMousePosition().x <= (WIDTH - restartTexture.width) / 2 + restartTexture.width) &&
                                            (GetMousePosition().y >= (HEIGHT - restartTexture.height) / 2 + 100 && GetMousePosition().y <= (HEIGHT - restartTexture.height) / 2 + 100 + restartTexture.height)))
            {
                gameState = PLAYING;
                score = 0;
                scrollMultiplier = 1.75;
                scrollIndex = 0;
                frameCounter = 0;
                positionComponents[dinoId].x = DINO_START_X_POS;
                positionComponents[dinoId].y = FLOOR_Y_POS;
                velocityComponents[dinoId].x = 0;
                velocityComponents[dinoId].y = 0;
                dinoComponents[dinoId].isDead = false;
                dinoComponents[dinoId].isDucking = false;
                dinoComponents[dinoId].isJumping = false;
                for (int i = 0; i < nextEntityId; i++)
                {
                    if (HasComponent(entities, i, OBSTACLE))
                    {
                        positionComponents[i].x = -1000;
                        UpdateObstacleTypeSystem(entities);
                        UpdateObstacleTextureSystem(entities, cactusLargeTexture, cactusSmallTexture, pterodactylTexture);
                        UpdateObstacleVelocity(i, 1.0f);
                        UpdateObstaclePosition(i, scrollIndex);
                    }
                    if (HasComponent(entities, i, CLOUD))
                    {
                        positionComponents[i].x = cloudComponents[i].xIndex * (cloudTexture.width + 20) + GetRandomValue(0, MAX_CLOUDS / 2) * WIDTH;
                        positionComponents[i].y = 30 + cloudComponents[i].yIndex * (cloudTexture.height + 20);
                    }
                }
            }
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (score > highScore)
        {
            highScore = score;
        }
        DrawScore(score, highScore, scoreTexture);
        DrawTextureEx(horizonTexture, (Vector2){scrollIndex, FLOOR_Y_POS + TREX_SPRITES_HEIGHT - 38}, 0.0f, 1.0f, WHITE);
        DrawTextureEx(horizonTexture, (Vector2){scrollIndex + horizonTexture.width, FLOOR_Y_POS + TREX_SPRITES_HEIGHT - 38}, 0.0f, 1.0f, WHITE);
        DrawSpriteSystem(entities);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(dinoTexture);
    UnloadTexture(dinoDuckTexture);
    UnloadTexture(horizonTexture);
    UnloadTexture(pterodactylTexture);
    UnloadTexture(restartTexture);
    UnloadTexture(cactusLargeTexture);
    UnloadTexture(cactusSmallTexture);
    UnloadTexture(cloudTexture);
    UnloadTexture(gameOverTexture);
    UnloadTexture(scoreTexture);

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
// ----------------------------------------------------------------------------------

// Animation + Frames Functions Definition
// ----------------------------------------------------------------------------------
void UpdateDinoAnimationSystem(Entity *entities, Texture2D dinoTexture, Texture2D dinoDuckTexture)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if (!HasComponent(entities, i, DINO))
            continue;
        if (!HasComponent(entities, i, ANIMATION))
            continue;
        if (!HasComponent(entities, i, SPRITE))
            continue;
        if (dinoComponents[i].isDead)
        {
            spriteComponents[i].texture = dinoTexture;
            spriteComponents[i].sourceRec.width = (float)TREX_SPRITES_WIDTH;
            spriteComponents[i].sourceRec.height = (float)TREX_SPRITES_HEIGHT;
            animationComponents[i].frameIndexSlice[0] = 4;
            animationComponents[i].frameIndexSlice[1] = 4;
        }
        else if (positionComponents[i].y < FLOOR_Y_POS)
        {
            spriteComponents[i].texture = dinoTexture;
            spriteComponents[i].sourceRec.width = (float)TREX_SPRITES_WIDTH;
            spriteComponents[i].sourceRec.height = (float)TREX_SPRITES_HEIGHT;
            animationComponents[i].frameIndexSlice[0] = 0;
            animationComponents[i].frameIndexSlice[1] = 0;
        }
        else if (dinoComponents[i].isDucking)
        {
            spriteComponents[i].texture = dinoDuckTexture;
            spriteComponents[i].sourceRec.width = (float)TREX_SPRITES_WIDTH_DUCK;
            spriteComponents[i].sourceRec.height = (float)TREX_SPRITES_HEIGHT_DUCK;
            animationComponents[i].frameIndexSlice[0] = 0;
            animationComponents[i].frameIndexSlice[1] = 1;
        }
        else
        {
            spriteComponents[i].texture = dinoTexture;
            spriteComponents[i].sourceRec.width = (float)TREX_SPRITES_WIDTH;
            spriteComponents[i].sourceRec.height = (float)TREX_SPRITES_HEIGHT;
            animationComponents[i].frameIndexSlice[0] = 2;
            animationComponents[i].frameIndexSlice[1] = 3;
        }
    }
}

void UpdateDinoPoseSystem(Entity *entities)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if (!HasComponent(entities, i, DINO))
            continue;
        if (!HasComponent(entities, i, POSITION))
            continue;
        dinoComponents[i].isJumping = IsJumping(positionComponents[i].y);
        dinoComponents[i].isDucking = IsDucking(positionComponents[i].y);
    }
}

void UpdatePositionSystem(Entity *entities, float scrollIndex)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if (!HasComponent(entities, i, POSITION))
            continue;
        if (!HasComponent(entities, i, VELOCITY))
            continue;

        positionComponents[i].x += velocityComponents[i].x;
        positionComponents[i].y += velocityComponents[i].y;

        if (HasComponent(entities, i, DINO))
        {
            UpdateDinoPosition(i);
        }

        if (HasComponent(entities, i, CLOUD))
        {
            UpdateCloudPosition(i, scrollIndex);
        }

        if (HasComponent(entities, i, OBSTACLE))
        {
            UpdateObstaclePosition(i, scrollIndex);
        }
    }
}

void UpdateVelocitySystem(Entity *entities, float scrollMultiplier)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if (!HasComponent(entities, i, VELOCITY))
            continue;
        if (!HasComponent(entities, i, POSITION))
            continue;
        if (HasComponent(entities, i, DINO))
        {
            UpdateDinoVelocity(i, scrollMultiplier);
        }
        if (HasComponent(entities, i, OBSTACLE))
        {
            UpdateObstacleVelocity(i, scrollMultiplier);
        }
        if (HasComponent(entities, i, CLOUD))
        {
            UpdateCloudVelocity(i, scrollMultiplier);
        }
    }
}

void UpdateDinoVelocity(int i, float scrollMultiplier)
{
    if (!dinoComponents[i].isJumping)
    {
        velocityComponents[i].y = 0;
        dinoComponents[i].jumpFrameCount = 0;
    }
    else
    {
        float multiplier = 0.25f * scrollMultiplier;
        float A = (float)(INITIAL_JUMP_VELOCITY - DROP_VELOCITY) * multiplier;
        float f = multiplier / (float)MIN_FRAME_SPEED;
        float phi = PI / 2;
        float t = dinoComponents[i].jumpFrameCount;
        velocityComponents[i].y =
            A *
            sin(2 * PI * f * t + phi);
        dinoComponents[i].jumpFrameCount++;
    }

    if (positionComponents[i].x < DINO_PLAY_X_POS &&
        (positionComponents[i].y == FLOOR_Y_POS || dinoComponents[i].isDucking))
    {
        velocityComponents[i].x =
            -sin(PI *
                 ((positionComponents[i].x -
                   (DINO_START_X_POS + DINO_PLAY_X_POS) / 2) /
                  (DINO_PLAY_X_POS - DINO_START_X_POS)));
    }
    else
    {
        velocityComponents[i].x = 0;
        dinoComponents[i].slideFrameCount = 0;
    }
}

void UpdateFrameCounterSystem(Entity *entities)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if ((entities[i].componentMask & ANIMATION) != ANIMATION)
            continue;
        animationComponents[i].framesCounter++;
        if (animationComponents[i].framesCounter >= (60 / animationComponents[i].framesSpeed))
        {
            animationComponents[i].framesCounter = 0;
        }
    }
}

void UpdateCurrentFrameIndexSystem(Entity *entities)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if ((entities[i].componentMask & ANIMATION) != ANIMATION)
            continue;
        if ((entities[i].componentMask & SPRITE) != SPRITE)
            continue;
        if (animationComponents[i].framesCounter == 0)
        {
            animationComponents[i].currentFrameIndex++;
            if (animationComponents[i].currentFrameIndex > animationComponents[i].frameIndexSlice[1] - animationComponents[i].frameIndexSlice[0])
                animationComponents[i].currentFrameIndex = 0;
        }
        spriteComponents[i].sourceRec.x = (float)spriteComponents[i].sourceRec.width * (float)(animationComponents[i].currentFrameIndex + animationComponents[i].frameIndexSlice[0]);
    }
}

void DrawSpriteSystem(Entity *entities)
{
    for (int i = nextEntityId - 1; i >= 0; i--)
    {
        if (!HasComponent(entities, i, SPRITE))
            continue;
        if (!HasComponent(entities, i, POSITION))
            continue;
        DrawTextureRec(spriteComponents[i].texture, spriteComponents[i].sourceRec, (Vector2){positionComponents[i].x, positionComponents[i].y}, WHITE);
    }
}

void UpdateCloudVelocity(int i, float scrollMultiplier)
{
    velocityComponents[i].x = -(1.5f * scrollMultiplier);
}

void UpdateObstacleVelocity(int i, float scrollMultiplier)
{
    velocityComponents[i].x = -(2.5f * scrollMultiplier);
}

void UpdateCloudPosition(int i, float scrollIndex)
{
    if (positionComponents[i].x < -spriteComponents[i].sourceRec.width)
    {
        positionComponents[i].x = cloudComponents[i].xIndex * (spriteComponents[i].sourceRec.width + 20) + GetRandomValue(0, MAX_CLOUDS / 2) * WIDTH + scrollIndex;
        positionComponents[i].y = 30 + cloudComponents[i].yIndex * (spriteComponents[i].sourceRec.height + 20);
    }
}

void UpdateDinoPosition(int i)
{
    if (positionComponents[i].y > FLOOR_Y_POS)
    {
        positionComponents[i].y = FLOOR_Y_POS;
    }
    if (IsDucking(positionComponents[i].y))
    {
        positionComponents[i].y = FLOOR_Y_POS + (TREX_SPRITES_HEIGHT - TREX_SPRITES_HEIGHT_DUCK);
    }
    if (dinoComponents[i].isDead)
    {
        positionComponents[i].y = FLOOR_Y_POS;
    }
    if (positionComponents[i].x > WIDTH / 2)
    {
        positionComponents[i].x = WIDTH / 2;
    }
}

void UpdateObstaclePosition(int i, float scrollIndex)
{
    if (IsOutOfBounds(i))
    {
        positionComponents[i].x = WIDTH +
                                  i * WIDTH / MAX_OBSTACLES +
                                  scrollIndex;
    }

    switch (obstacleComponents[i].type)
    {
    case CACTUS_LARGE:
        positionComponents[i].y = FLOOR_Y_POS - 15;
        break;
    case CACTUS_SMALL:
        positionComponents[i].y = FLOOR_Y_POS + 10;
        break;
    case PTERODACTYL:
        positionComponents[i].y = FLOOR_Y_POS - 60;
        break;
    }
}

void UpdateObstacleTypeSystem(Entity *entities)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if (!HasComponent(entities, i, OBSTACLE))
            continue;
        if (!IsOutOfBounds(i))
            continue;
        obstacleComponents[i].type = GetRandomValue(0, 2);
    }
}

void UpdateObstacleTextureSystem(Entity *entities, Texture2D cactusLargeTexture, Texture2D cactusSmallTexture, Texture2D pterodactylTexture)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if (!HasComponent(entities, i, OBSTACLE))
            continue;
        if (!HasComponent(entities, i, SPRITE))
            continue;
        if (!IsOutOfBounds(i))
            continue;
        switch (obstacleComponents[i].type)
        {
        case CACTUS_LARGE:
            RemoveComponent(entities, i, ANIMATION);
            spriteComponents[i].texture = cactusLargeTexture;
            int spriteOffsetLargeCactus = GetRandomValue(0, 3);
            int clusterSizeLargeCactus = GetRandomValue(1, 2);
            spriteComponents[i].sourceRec = (Rectangle){cactusLargeTexture.width / 6.0f * (float)spriteOffsetLargeCactus, 0, (float)cactusLargeTexture.width / 6.0f * (float)clusterSizeLargeCactus, (float)cactusLargeTexture.height};
            break;
        case CACTUS_SMALL:
            RemoveComponent(entities, i, ANIMATION);
            spriteComponents[i].texture = cactusSmallTexture;
            int spriteOffsetSmallCactus = GetRandomValue(0, 6);
            int clusterSizeSmallCactus = GetRandomValue(1, 2);
            spriteComponents[i].sourceRec = (Rectangle){cactusSmallTexture.width / 6.0f * (float)spriteOffsetSmallCactus, 0, (float)cactusSmallTexture.width / 6.0f * (float)clusterSizeSmallCactus, (float)cactusSmallTexture.height};
            break;
        case PTERODACTYL:
            AddComponent(entities, i, ANIMATION);
            animationComponents[i] = (AnimationComponent){0, {0, 1}, 3, 0};
            spriteComponents[i].texture = pterodactylTexture;
            spriteComponents[i].sourceRec = (Rectangle){0, 0, (float)pterodactylTexture.width / 2, (float)pterodactylTexture.height};
            break;
        }
    }
}

void UpdateCollisionSystem(Entity *entities)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if (!HasComponent(entities, i, COLLISION))
            continue;
        if (!HasComponent(entities, i, POSITION))
            continue;
        if (!HasComponent(entities, i, OBSTACLE))
            continue;
        for (int j = 0; j < nextEntityId; j++)
        {
            if (!HasComponent(entities, j, DINO))
                continue;
            if (!IsSpriteOverlap(
                    (Rectangle){positionComponents[i].x,
                                positionComponents[i].y,
                                spriteComponents[i].sourceRec.width,
                                spriteComponents[i].sourceRec.height},
                    (Rectangle){positionComponents[j].x,
                                positionComponents[j].y,
                                spriteComponents[j].sourceRec.width,
                                spriteComponents[j].sourceRec.height}))
                continue;
            if (!IsCollisionMaskOverlap(entities, i, j))
                continue;
            dinoComponents[j].isDead = true;
            break;
        }
    }
}

bool IsCollisionMaskOverlap(Entity *entities, int i, int j)
{
    CollisionMask mask1 = GetCollisionMaskFromSprite(entities, i);
    CollisionMask mask2 = GetCollisionMaskFromSprite(entities, j);

    int xStart = (int)positionComponents[i].x - (int)positionComponents[j].x;
    int yStart = (int)positionComponents[i].y - (int)positionComponents[j].y;
    int xEnd = xStart + mask1.width;
    int yEnd = yStart + mask1.height;

    for (int x = xStart; x < xEnd; x++)
    {
        for (int y = yStart; y < yEnd; y++)
        {
            if (x < 0 || x >= mask2.width || y < 0 || y >= mask2.height)
                continue;
            if (mask1.pixels[(x - xStart) + (y - yStart) * mask1.width] == 1 && mask2.pixels[x + y * mask2.width] == 1)
            {
                free(mask1.pixels);
                free(mask2.pixels);
                return true;
            }
        }
    }

    free(mask1.pixels);
    free(mask2.pixels);
    return false;
}

CollisionMask GetCollisionMaskFromSprite(Entity *entities, int i)
{
    Image image = LoadImageFromTexture(spriteComponents[i].texture);
    ImageCrop(&image, (Rectangle){spriteComponents[i].sourceRec.x, spriteComponents[i].sourceRec.y, spriteComponents[i].sourceRec.width, spriteComponents[i].sourceRec.height});
    CollisionMask collisionMask = (CollisionMask){image.width, image.height, malloc(image.width * image.height)};
    for (int x = 0; x < image.width; x++)
    {
        for (int y = 0; y < image.height; y++)
        {
            Color pixelColor = GetImageColor(image, x, y);
            if (pixelColor.a == 0)
            {
                collisionMask.pixels[x + y * image.width] = 0;
                continue;
            }
            collisionMask.pixels[x + y * image.width] = 1;
        }
    }
    UnloadImage(image);
    return collisionMask;
}

// ----------------------------------------------------------------------------------

// Helper Functions Definition
// ----------------------------------------------------------------------------------
bool IsJumping(float y)
{
    if (IsKeyPressed(KEY_SPACE))
    {
        return true;
    }
    if (IsKeyPressed(KEY_UP))
    {
        return true;
    }
    if (y < FLOOR_Y_POS)
    {
        return true;
    }
    return false;
}

bool IsDucking(int posY)
{
    if (posY < FLOOR_Y_POS)
    {
        return false;
    }
    if (IsKeyDown(KEY_DOWN))
    {
        return true;
    }
    return false;
}

bool IsSpriteOverlap(Rectangle rec1, Rectangle rec2)
{
    if (CheckCollisionRecs(rec1, rec2))
    {
        return true;
    }
    return false;
}

bool IsOutOfBounds(int i)
{
    if (positionComponents[i].x < -spriteComponents[i].sourceRec.width - 50)
    {
        return true;
    }
    return false;
}

void DrawScore(int score, int highScore, Texture2D scoreTexture)
{
    DrawText(TextFormat("%i", score), 50, 10, 20, BLACK);
    DrawText(TextFormat("HI %i", highScore), 120, 10, 20, BLACK);
}

int LoadHighScore()
{
    int highScore = 0;
    unsigned int dataSize = 0;
    unsigned char *fileData = LoadFileData("highscore.txt", &dataSize);
    if (fileData != NULL)
    {
        int *dataPtr = (int *)fileData;
        highScore = *dataPtr;
    }
    UnloadFileData(fileData);
    return highScore;
}

void SaveHighScore(int highScore)
{
    unsigned int dataSize = 0;
    unsigned int newDatasize = 0;
    unsigned char *fileData = LoadFileData("highscore.txt", &dataSize);
    unsigned char *newFileData = NULL;

    if (fileData != NULL)
    {
        if (dataSize <= sizeof(int))
        {
            newDatasize = sizeof(int);
            newFileData = (unsigned char *)realloc(fileData, newDatasize);

            if (newFileData != NULL)
            {
                int *dataPtr = (int *)newFileData;
                *dataPtr = highScore;
            }
        }
        else
        {
            newDatasize = dataSize;
            newFileData = fileData;
            int *dataPtr = (int *)newFileData;
            *dataPtr = highScore;
        }
        RL_FREE(newFileData);
    }
    else
    {
        dataSize = sizeof(int);
        fileData = (unsigned char *)malloc(dataSize);
        int *dataPtr = (int *)fileData;
        *dataPtr = highScore;
        SaveFileData("highscore.txt", fileData, dataSize);
        UnloadFileData(fileData);
    }
}
// ----------------------------------------------------------------------------------

// Entity Component System: Functions
// ----------------------------------------------------------------------------------
Entity CreateEntity()
{
    Entity e = {nextEntityId++, 0};
    return e;
}

bool HasComponent(Entity *entities, int id, int component)
{
    return entities[id].componentMask & component;
}

void AddComponent(Entity *entities, int id, int component)
{
    entities[id].componentMask |= component;
}

void RemoveComponent(Entity *entities, int id, int component)
{
    entities[id].componentMask &= ~component;
}
// ----------------------------------------------------------------------------------