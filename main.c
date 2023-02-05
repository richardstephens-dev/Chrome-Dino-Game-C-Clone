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
const int HEIGHT = 480;
const int WIDTH = 800;
const int TREX_SPRITES_WIDTH = 88;
const int TREX_SPRITES_HEIGHT = 94;
const int TREX_SPRITES_WIDTH_DUCK = 88;
const int TREX_SPRITES_HEIGHT_DUCK = 94;
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
const float FLOOR_Y_POS = 280.0f;
const float JUMP_Y_POS = 100.0f;
const float INITIAL_JUMP_VELOCITY = -24.0f;
const float DROP_VELOCITY = 12.0f;
const float DINO_START_X_POS = 250.0f;
const float DINO_PLAY_X_POS = WIDTH / 2 + TREX_SPRITES_WIDTH;

enum ComponentsEnum
{
    SIZE = 0b00000001,
    POSITION = 0b00000010,
    VELOCITY = 0b00000100,
    SPRITE = 0b00001000,
    ANIMATION = 0b00010000,
    DINO = 0b00100000,
    COLLISION = 0b01000000,
    OBSTACLE = 0b10000000,
    CLOUD = 0b100000000,
};

enum GameState
{
    MENU,
    PLAYING,
    GAMEOVER
};

// non const/macro variables
int nextEntityId = 0;
//----------------------------------------------------------------------------------

// Entity Component System
// ----------------------------------------------------------------------------------
typedef struct SizeComponent
{
    int width, height;
} SizeComponent;
SizeComponent sizeComponents[MAX_ENTITIES];

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
    bool isObstacle;
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
bool IsJumping(float y);
Entity CreateEntity();
void UpdateDinoAnimationSystem(Entity *entities);
void UpdateDinoPositionSystem(Entity *entities);
void UpdateDinoPoseSystem(Entity *entities);
void UpdateDinoVelocitySystem(Entity *entities);
void UpdateFrameCounterSystem(Entity *entities);
void UpdateCurrentFrameIndexSystem(Entity *entities);
bool IsDucking(int posY);
bool HasComponent(Entity *entities, int id, int component);
void AddComponent(Entity *entities, int id, int component);
void DrawSpriteSystem(Entity *entities);
void UpdateCloudSystem(Entity *entities);

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

    Image horizonImage = LoadImage("resources/horizon.png");
    Texture2D horizonTexture = LoadTextureFromImage(horizonImage);
    UnloadImage(horizonImage);

    Image pterodactylImage = LoadImage("resources/ptero.png");
    Texture2D pterodactylTexture = LoadTextureFromImage(pterodactylImage);
    UnloadImage(pterodactylImage);

    Image restartImage = LoadImage("resources/restart.png");
    Texture2D restartTexture = LoadTextureFromImage(restartImage);
    UnloadImage(restartImage);

    Image cactusLargeImage = LoadImage("resources/cactus-large.png");
    Texture2D cactusLargeTexture = LoadTextureFromImage(cactusLargeImage);
    UnloadImage(cactusLargeImage);

    Image cactusSmallImage = LoadImage("resources/cactus-small.png");
    Texture2D cactusSmallTexture = LoadTextureFromImage(cactusSmallImage);
    UnloadImage(cactusSmallImage);

    Image cloudImage = LoadImage("resources/cloud.png");
    Texture2D cloudTexture = LoadTextureFromImage(cloudImage);
    UnloadImage(cloudImage);

    Entity entities[MAX_ENTITIES];
    int dinoId = nextEntityId;
    entities[dinoId] = CreateEntity();
    AddComponent(entities, dinoId, SIZE);
    AddComponent(entities, dinoId, POSITION);
    AddComponent(entities, dinoId, VELOCITY);
    AddComponent(entities, dinoId, SPRITE);
    AddComponent(entities, dinoId, ANIMATION);
    AddComponent(entities, dinoId, DINO);
    AddComponent(entities, dinoId, COLLISION);
    sizeComponents[dinoId] = (SizeComponent){dinoTexture.width / 6, dinoTexture.height};
    positionComponents[dinoId] = (PositionComponent){DINO_START_X_POS, FLOOR_Y_POS};
    velocityComponents[dinoId] = (VelocityComponent){0.0f, 0.0f};
    spriteComponents[dinoId] = (SpriteComponent){dinoTexture, {0.0f, 0.0f, (float)dinoTexture.width / 6, (float)dinoTexture.height}};
    animationComponents[dinoId] = (AnimationComponent){0, {2, 3}, 8, 0};
    dinoComponents[dinoId] = (DinoComponent){false, false, false, 0, 0};
    collisionComponents[dinoId] = (CollisionComponent){(Rectangle){positionComponents[dinoId].x, positionComponents[dinoId].y, sizeComponents[dinoId].width, sizeComponents[dinoId].height}};

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
        positionComponents[cloudId].x = i * cloudTexture.width + GetRandomValue(0, WIDTH * MAX_CLOUDS / 2);
        positionComponents[cloudId].y = i * (cloudTexture.height + 10);
    }

    int frameCounter = 0;
    int score = 0;
    float scrollMultiplier = 1;
    float scrollIndex = 0;

    SetTargetFPS(60);
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
            UpdateDinoAnimationSystem(entities);
            UpdateDinoPositionSystem(entities);
            UpdateDinoPoseSystem(entities);
            UpdateDinoVelocitySystem(entities);
            UpdateFrameCounterSystem(entities);
            UpdateCurrentFrameIndexSystem(entities);
            UpdateCloudSystem(entities);
            //----------------------------------------------------------------------------------

            // Update game variables
            //----------------------------------------------------------------------------------
            frameCounter++;
            scrollIndex -= 2.5f * scrollMultiplier;
            scrollMultiplier *= 1.0001f;
            if (scrollIndex <= -horizonTexture.width)
            {
                scrollIndex = 0;
            }
            if (frameCounter % 10 == 0)
            {
                score += 1 * scrollMultiplier;
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
    UnloadTexture(horizonTexture);
    UnloadTexture(pterodactylTexture);
    UnloadTexture(restartTexture);
    UnloadTexture(cactusLargeTexture);
    UnloadTexture(cactusSmallTexture);
    UnloadTexture(cloudTexture);

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
// ----------------------------------------------------------------------------------

// Animation + Frames Functions Definition
// ----------------------------------------------------------------------------------
void UpdateDinoAnimationSystem(Entity *entities)
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
            spriteComponents[i].sourceRec.width = (float)TREX_SPRITES_WIDTH;
            spriteComponents[i].sourceRec.height = (float)TREX_SPRITES_HEIGHT;
            animationComponents[i].frameIndexSlice[0] = 0;
            animationComponents[i].frameIndexSlice[1] = 0;
        }
        else if (dinoComponents[i].isDucking)
        {
            spriteComponents[i].sourceRec.width = (float)TREX_SPRITES_WIDTH_DUCK;
            spriteComponents[i].sourceRec.height = (float)TREX_SPRITES_HEIGHT_DUCK;
            animationComponents[i].frameIndexSlice[0] = 1;
            animationComponents[i].frameIndexSlice[1] = 1;
        }
        else
        {
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

void UpdateDinoPositionSystem(Entity *entities)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if (!HasComponent(entities, i, DINO))
            continue;
        if (!HasComponent(entities, i, POSITION))
            continue;
        if (!HasComponent(entities, i, VELOCITY))
            continue;
        positionComponents[i].x += velocityComponents[i].x;
        positionComponents[i].y += velocityComponents[i].y;
        if (positionComponents[i].y > FLOOR_Y_POS)
        {
            positionComponents[i].y = FLOOR_Y_POS;
        }
        if (positionComponents[i].x > WIDTH / 2)
        {
            positionComponents[i].x = WIDTH / 2;
        }
    }
}

void UpdateDinoVelocitySystem(Entity *entities)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if (!HasComponent(entities, i, DINO))
            continue;
        if (!HasComponent(entities, i, VELOCITY))
            continue;
        if (!HasComponent(entities, i, POSITION))
            continue;
        if (!dinoComponents[i].isJumping)
        {
            velocityComponents[i].y = 0;
            dinoComponents[i].jumpFrameCount = 0;
        }
        else
        {
            velocityComponents[i].y =
                (INITIAL_JUMP_VELOCITY - DROP_VELOCITY) /
                    pow((INITIAL_JUMP_VELOCITY - 2 * MIN_FRAME_SPEED), 2) *
                    pow((dinoComponents[i].jumpFrameCount - 2 * MIN_FRAME_SPEED), 2) +
                DROP_VELOCITY;
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

void UpdateCloudSystem(Entity *entities)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        if (!HasComponent(entities, i, CLOUD))
            continue;
        if (!HasComponent(entities, i, POSITION))
            continue;
        if (!HasComponent(entities, i, VELOCITY))
            continue;
        positionComponents[i].x += velocityComponents[i].x;
        if (positionComponents[i].x < -spriteComponents[i].sourceRec.width)
        {
            velocityComponents[i].x = GetRandomValue(1, 1.5);
            positionComponents[i].x = GetRandomValue(WIDTH, WIDTH * MAX_CLOUDS);
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
// ----------------------------------------------------------------------------------

// Utils
// ----------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------