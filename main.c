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
//----------------------------------------------------------------------------------

// Local Variables Definition (local to this module)
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
const float GRAVITY = 0.6;
const int INIITAL_JUMP_VELOCITY = 12;
const int INVERT_FADE_DURATION = 12000;
const int INVERT_DISTANCE = 700;
const int MAX_BLINK_COUNT = 3;
const int MAX_CLOUDS = 6;
const int MAX_OBSTACLE_LENGTH = 3;
const int MAX_OBSTACLE_DUPLICATION = 2;
const int MAX_SPEED = 13;
const int MIN_JUMP_HEIGHT = 35;
const int SPEED = 6;
const int SPEED_DROP_COEFFICIENT = 3;
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
const enum ComponentsEnum {
    SIZE = 0,
    POSITION = 1,
    VELOCITY = 2,
    SPRITE = 3,
    ANIMATION = 4,
    DINO = 5,
    COLLISION = 6,
    OBSTACLE = 7
};

// non const/macro variables
int nextEntityId = 0;
//----------------------------------------------------------------------------------

// Entity Component System: typedefs
// ----------------------------------------------------------------------------------
// everything except GUI should go in here
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

typedef struct Entity
{
    int id;
    int component_mask;
} Entity;
//----------------------------------------------------------------------------------

// Local Functions Declaration
//----------------------------------------------------------------------------------
bool IsJumping(float dinoY, bool isJumping);
float UpdateDinoY(float dinoY, bool isJumping);
float UpdateDinoX(float dinoX);
Entity CreateEntity();
void AddComponent(Entity e, int component);
bool HasComponent(Entity e, int component);
void UpdateDinoSystem(Entity *entities);
void UpdateFrameCounterSystem(Entity *entities);
void UpdateCurrentFrameIndexSystem(Entity *entities);
bool IsDucking(int posY);

//----------------------------------------------------------------------------------

// Entity Component System: Functions
// ----------------------------------------------------------------------------------
Entity CreateEntity()
{
    Entity e = {nextEntityId++, 0};
    return e;
}

void AddComponent(Entity e, int component)
{
    if (component == SIZE)
    {
        sizeComponents[e.id] = (SizeComponent){0};
    }
    if (component == POSITION)
    {
        positionComponents[e.id] = (PositionComponent){0};
    }
    if (component == VELOCITY)
    {
        velocityComponents[e.id] = (VelocityComponent){0};
    }
    if (component == SPRITE)
    {
        spriteComponents[e.id] = (SpriteComponent){0};
    }
    if (component == ANIMATION)
    {
        animationComponents[e.id] = (AnimationComponent){0};
    }
    if (component == DINO)
    {
        dinoComponents[e.id] = (DinoComponent){0};
    }
    if (component == COLLISION)
    {
        collisionComponents[e.id] = (CollisionComponent){0};
    }
    if (component == OBSTACLE)
    {
        obstacleComponents[e.id] = (ObstacleComponent){0};
    }
    e.component_mask |= component;
}

bool HasComponent(Entity e, int component)
{
    return e.component_mask & component;
}

// ----------------------------------------------------------------------------------

// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    // TODO: adjust dimensions on resize
    InitWindow(WIDTH, HEIGHT, "Dino Game");

    // images
    Image image = LoadImage("resources/dino.png");
    Texture2D dinoTexture = LoadTextureFromImage(image);
    UnloadImage(image);

    // horizon: clouds, obstacles, ground

    //
    /*Rectangle dinoFrameRec = {0.0f, 0.0f, (float)dinoTexture.width / 6, (float)dinoTexture.height};
    int dinoCurrentFrame = 0;
    int dinoFrameCounter = 0;
    int frameSpeed = 8; // Number of spritesheet frames shown by second
    float dinoY = 280.0f;
    float dinoX = 250.0f;
    int dinoAnimationIndexSlice[2] = {0, 0};
    bool isJumping = false;
    bool isDucking = false;
    Vector2 dinoPos = {dinoX, dinoY};*/

    // set up dino entity
    // make array of entities
    Entity entities[MAX_ENTITIES];
    Entity dino = CreateEntity();
    entities[dino.id] = dino;
    AddComponent(dino, SIZE);
    AddComponent(dino, POSITION);
    AddComponent(dino, VELOCITY);
    AddComponent(dino, SPRITE);
    AddComponent(dino, ANIMATION);
    AddComponent(dino, DINO);
    AddComponent(dino, COLLISION);
    sizeComponents[dino.id] = (SizeComponent){dinoTexture.width / 6, dinoTexture.height};
    positionComponents[dino.id] = (PositionComponent){250.0f, 280.0f};
    velocityComponents[dino.id] = (VelocityComponent){0.0f, 0.0f};
    spriteComponents[dino.id] = (SpriteComponent){dinoTexture, {0.0f, 0.0f, (float)dinoTexture.width / 6, (float)dinoTexture.height}};
    animationComponents[dino.id] = (AnimationComponent){0, {2, 3}, 8, 0};
    dinoComponents[dino.id] = (DinoComponent){false, false, false};
    collisionComponents[dino.id] = (CollisionComponent){(Rectangle){positionComponents[dino.id].x, positionComponents[dino.id].y, sizeComponents[dino.id].width, sizeComponents[dino.id].height}};

    // show dinoTexture width / 6 and height

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Wait for game to start
    //--------------------------------------------------------------------------------------
    while (!IsKeyPressed(KEY_ENTER) && !WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Press ENTER to start!", 200, 200, 20, PINK);
        EndDrawing();
    }
    //--------------------------------------------------------------------------------------

    // Main game loop
    //--------------------------------------------------------------------------------------
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update Systems
        //----------------------------------------------------------------------------------
        UpdateDinoSystem(entities);
        UpdateFrameCounterSystem(entities);
        UpdateCurrentFrameIndexSystem(entities);
        positionComponents[dino.id].x = UpdateDinoX(positionComponents[dino.id].x);
        positionComponents[dino.id].y = UpdateDinoY(positionComponents[dino.id].y, dinoComponents[dino.id].isJumping);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText(TextFormat("%02i FPS", GetFPS()), 575, 210, 50, PINK);
        DrawTextureRec((Texture2D)spriteComponents[dino.id].texture, (Rectangle)spriteComponents[dino.id].sourceRec, (Vector2){positionComponents[dino.id].x, positionComponents[dino.id].y}, WHITE);
        DrawText(TextFormat("Dino X: %i", (int)positionComponents[dino.id].x), 575, 10, 20, PINK);
        DrawText(TextFormat("Dino Y: %i", (int)positionComponents[dino.id].y), 575, 30, 20, PINK);
        DrawText(TextFormat("Is Jumping: %s", dinoComponents[dino.id].isJumping ? "true" : "false"), 575, 70, 20, PINK);
        DrawText(TextFormat("Is Crouching: %s", dinoComponents[dino.id].isDucking ? "true" : "false"), 575, 90, 20, PINK);
        DrawText(TextFormat("Animation Slice: %i %i", animationComponents[dino.id].frameIndexSlice[0], animationComponents[dino.id].frameIndexSlice[1]), 575, 130, 20, PINK);
        DrawText(TextFormat("Current Frame: %i", animationComponents[dino.id].currentFrameIndex), 575, 150, 20, PINK);
        DrawText(TextFormat("Frame Counter: %i", animationComponents[dino.id].framesCounter), 575, 170, 20, PINK);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(dinoTexture); // Unload dinoTexture
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
// ----------------------------------------------------------------------------------

// Animation + Frames Functions Definition
// ----------------------------------------------------------------------------------
void UpdateDinoSystem(Entity *entities)
{
    for (int i = 0; i < nextEntityId; i++)
    {
        dinoComponents[i].isJumping = IsJumping(positionComponents[i].y, dinoComponents[i].isJumping);
        dinoComponents[i].isDucking = IsDucking(positionComponents[i].y);
        if (positionComponents[i].y < 280)
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

void UpdateFrameCounterSystem(Entity *entities)
{
    for (int i = 0; i < nextEntityId; i++)
    {
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
        if (animationComponents[i].framesCounter == 0)
        {
            animationComponents[i].currentFrameIndex++;
            if (animationComponents[i].currentFrameIndex > animationComponents[i].frameIndexSlice[1] - animationComponents[i].frameIndexSlice[0])
                animationComponents[i].currentFrameIndex = 0;
        }
        spriteComponents[i].sourceRec.x = (float)spriteComponents[i].sourceRec.width * (float)(animationComponents[i].currentFrameIndex + animationComponents[i].frameIndexSlice[0]);
    }
}

// ----------------------------------------------------------------------------------

// Input Functions Definition
// ----------------------------------------------------------------------------------
bool IsJumping(float dinoY, bool isJumping)
{
    if (dinoY <= 100.0f)
    {
        return false;
    }
    if (isJumping)
    {
        return true;
    }
    if (dinoY != 280.0f)
    {
        return false;
    }
    if (IsKeyPressed(KEY_SPACE))
    {
        return true;
    }
    if (IsKeyPressed(KEY_UP))
    {
        return true;
    }
    return false;
}

bool IsDucking(int posY)
{
    if (posY < 280.0f)
    {
        return false;
    }
    if (IsKeyDown(KEY_DOWN))
    {
        return true;
    }
    return false;
}
// ----------------------------------------------------------------------------------

// Collision Functions Definition
// ----------------------------------------------------------------------------------
bool IsColliding(Rectangle rec1, Rectangle rec2)
{
    if (CheckCollisionRecs(rec1, rec2))
    {
        return true;
    }
    return false;
}
// ----------------------------------------------------------------------------------

// Dino Movement Functions Definition
// ----------------------------------------------------------------------------------
float UpdateDinoY(float dinoY, bool isJumping)
{
    if (isJumping)
    {
        dinoY -= 5.0f;
    }
    else if (dinoY < 280.0f)
    {
        dinoY += 5.0f;
    }
    else
    {
        dinoY = 280.0f;
    }
    return dinoY;
}

float UpdateDinoX(float dinoX)
{
    return dinoX;
}