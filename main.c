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
const float ACCELERATION = 0.001;
const float BG_CLOUD_SPEED = 0.2;
const int BOTTOM_PAD = 10;
const int CLEAR_TIME = 3000;
const float CLOUD_FREQUENCY = 0.5;
const int GAMEOVER_CLEAR_TIME = 750;
const float GAP_COEFFICIENT = 0.6;
const int INIITAL_JUMP_VELOCITY = 12;
const int INVERT_FADE_DURATION = 12000;
const int INVERT_DISTANCE = 700;
const int MAX_BLINK_COUNT = 3;
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
const int MOON_SPRITE_X = 954;
const int MOON_SPRITE_Y = 2;
const int PTERODACTYL_SPRITE_X = 260;
const int PTERODACTYL_SPRITE_Y = 2;
const int PTERODACTYL_SPRITE_WIDTH = 46;
const int PTERODACTYL_SPRITE_HEIGHT = 40;
const int RESTART_SPRITE_X = 2;
const int RESTART_SPRITE_Y = 2;
const int RESTART_SPRITE_WIDTH = 46;
const int RESTART_SPRITE_HEIGHT = 40;
const int TEXT_SPRITE_X = 1294;
const int TEXT_SPRITE_Y = 2;
const int TEXT_SPRITE_WIDTH = 192;
const int TEXT_SPRITE_HEIGHT = 11;
const int STAR_SPRITE_X = 1276;
const int STAR_SPRITE_Y = 2;
const int JUMP_KEY = 38;
const int JUMP_KEY_ALT = 32;
const int DUCK_KEY = 40;
const int RESTART_KEY = 13;
const float FLOOR_Y_POS = 0.8f * HEIGHT;
const float JUMP_Y_POS = 100.0f;
const float INITIAL_JUMP_VELOCITY = -24.0f;
const float DROP_VELOCITY = 12.0f;
const float DINO_START_X_POS = 250.0f;
const float DINO_PLAY_X_POS = WIDTH / 2 + TREX_SPRITES_WIDTH;
const int MAX_OBSTACLES = 3;

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
//----------------------------------------------------------------------------------

// Local Functions Declaration
//----------------------------------------------------------------------------------
Entity CreateEntity();
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

bool IsJumping(float y);
bool IsDucking(int posY);
bool IsColliding(Rectangle rec1, Rectangle rec2);
bool IsOutOfBounds(int i);
//----------------------------------------------------------------------------------

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
    float scrollMultiplier = 1;
    float scrollIndex = 0;

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
        velocityComponents[cloudId].x = -GetRandomValue(1, 1.5);
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
            UpdateCollisionSystem(entities);
            UpdateObstacleTextureSystem(entities, cactusLargeTexture, cactusSmallTexture, pterodactylTexture);
            //----------------------------------------------------------------------------------

            // Update game variables
            //----------------------------------------------------------------------------------
            frameCounter++;
            scrollIndex -= 2.5f * scrollMultiplier;
            scrollMultiplier *= 1.00015f;
            if (scrollIndex <= -horizonTexture.width)
            {
                scrollIndex = 0;
            }
            if (frameCounter % 10 == 0)
            {
                score += 1 * scrollMultiplier;
            }

            if (dinoComponents[dinoId].isDead)
            {
                gameState = GAMEOVER;
            }

            //----------------------------------------------------------------------------------

            // Temporary / Testing / Debug
            //----------------------------------------------------------------------------------
            DrawText(TextFormat("FPS: %i", GetFPS()), 400, 10, 20, BLACK);
            DrawText(TextFormat("%i", score), 10, 10, 20, BLACK);
            //----------------------------------------------------------------------------------
        }

        if (gameState == GAMEOVER)
        {
            DrawTexture(gameOverTexture, (WIDTH - gameOverTexture.width) / 2, (HEIGHT - gameOverTexture.height) / 2, WHITE);
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);
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
        if (positionComponents[i].y < FLOOR_Y_POS)
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

    if (positionComponents[i].x < DINO_PLAY_X_POS && positionComponents[i].y == FLOOR_Y_POS)
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
            int clusterSizeLargeCactus = GetRandomValue(1, 3);
            spriteComponents[i].sourceRec = (Rectangle){cactusLargeTexture.width / 6.0f * (float)spriteOffsetLargeCactus, 0, (float)cactusLargeTexture.width / 6.0f * (float)clusterSizeLargeCactus, (float)cactusLargeTexture.height};
            break;
        case CACTUS_SMALL:
            RemoveComponent(entities, i, ANIMATION);
            spriteComponents[i].texture = cactusSmallTexture;
            int spriteOffsetSmallCactus = GetRandomValue(0, 6);
            int clusterSizeSmallCactus = GetRandomValue(1, 3);
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
        collisionComponents[i].collisionRec.x = positionComponents[i].x + 0.4f * spriteComponents[i].sourceRec.width;
        collisionComponents[i].collisionRec.y = positionComponents[i].y + 0.4f * spriteComponents[i].sourceRec.height;
        collisionComponents[i].collisionRec.width = spriteComponents[i].sourceRec.width * 0.6f;
        collisionComponents[i].collisionRec.height = spriteComponents[i].sourceRec.height * 0.6f;
        for (int j = 0; j < nextEntityId; j++)
        {
            if (!HasComponent(entities, j, DINO))
                continue;
            collisionComponents[j].collisionRec.x = positionComponents[j].x + 0.05f * spriteComponents[j].sourceRec.width;
            collisionComponents[j].collisionRec.y = positionComponents[j].y + 0.05f * spriteComponents[j].sourceRec.height;
            collisionComponents[j].collisionRec.width = spriteComponents[j].sourceRec.width * 0.9f;
            collisionComponents[j].collisionRec.height = spriteComponents[j].sourceRec.height * 0.9f;
            if (HasComponent(entities, j, DINO) && IsColliding(collisionComponents[i].collisionRec, collisionComponents[j].collisionRec))
            {
                dinoComponents[j].isDead = true;
                break;
            }
        }
    }
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

bool IsColliding(Rectangle rec1, Rectangle rec2)
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

// ----------------------------------------------------------------------------------

// Utils
// ----------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------