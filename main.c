/*******************************************************************************************
 *
 *   Scope & Goals:
 *   This is a C port of the Dino Game from the Google Chrome browser.
 *   This game is a work in progress and is not yet complete.
 *   The goal of this project is to learn C.
 *
 *   Dependencies:
 *   This game uses raylib. raylib is licensed wit the zlib/libpng license (www.raylib.com)
 *
 *   Copyright & License:
 *   This game is licensed under the ISC License (github.com/richardstephens-dev/chrome-dino-game-c-clone)
 *   Copyright (c) 2023 Richard J Stephens
 *
 ********************************************************************************************/

// Includes
//----------------------------------------------------------------------------------
#include "raylib.h"
//----------------------------------------------------------------------------------

// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
#define MAX_FRAME_SPEED 15
#define MIN_FRAME_SPEED 1
//----------------------------------------------------------------------------------

// Local Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDinoAnimationIndexSlice(bool isCrouching, int dinoY, int dinoAnimationIndexSlice[]);
int UpdateFrameCounter(int dinoFrameCounter, int frameSpeed, int dinoCurrentFrame, Rectangle dinoFrameRec);
int UpdateCurrentFrame(int dinoCurrentFrame, int dinoFrameCounter, int dinoAnimationIndexSlice[]);
Rectangle UpdateFrameRec(int dinoCurrentFrame, Rectangle dinoFrameRec, int dinoAnimationIndexSlice[]);
int UpdateFrameSpeed(int frameSpeed);
bool IsJumping(float dinoY, bool isJumping);
float UpdateDinoY(float dinoY, bool isJumping);
float UpdateDinoX(float dinoX);
//----------------------------------------------------------------------------------

// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Dino Game");

    // dino stuff
    Image image = LoadImage("resources/dino.png");
    Texture2D dinoTexture = LoadTextureFromImage(image);
    UnloadImage(image);
    Rectangle dinoFrameRec = {0.0f, 0.0f, (float)dinoTexture.width / 6, (float)dinoTexture.height};
    int dinoCurrentFrame = 0;
    int dinoFrameCounter = 0;
    int frameSpeed = 8; // Number of spritesheet frames shown by second
    float dinoY = 280.0f;
    float dinoX = 250.0f;
    int dinoAnimationIndexSlice[2] = {0, 0};
    bool isJumping = false;
    bool isCrouching = false;
    Vector2 dinoPos = {dinoX, dinoY};

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
        // Update
        //----------------------------------------------------------------------------------
        UpdateDinoAnimationIndexSlice(isCrouching, dinoY, dinoAnimationIndexSlice);
        dinoFrameCounter = UpdateFrameCounter(dinoFrameCounter, frameSpeed, dinoCurrentFrame, dinoFrameRec);
        dinoCurrentFrame = UpdateCurrentFrame(dinoCurrentFrame, dinoFrameCounter, dinoAnimationIndexSlice);
        dinoFrameRec = UpdateFrameRec(dinoCurrentFrame, dinoFrameRec, dinoAnimationIndexSlice);
        frameSpeed = UpdateFrameSpeed(frameSpeed);
        isJumping = IsJumping(dinoY, isJumping);
        dinoY = UpdateDinoY(dinoY, isJumping);
        dinoPos = (Vector2){dinoX, dinoY};
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText(TextFormat("%02i FPS", GetFPS()), 575, 210, 50, PINK);
        DrawTextureRec(dinoTexture, dinoFrameRec, dinoPos, WHITE);
        DrawText(TextFormat("Dino X: %i", (int)dinoX), 575, 10, 20, PINK);
        DrawText(TextFormat("Dino Y: %i", (int)dinoY), 575, 30, 20, PINK);
        DrawText(TextFormat("Is Jumping: %s", isJumping ? "true" : "false"), 575, 70, 20, PINK);
        DrawText(TextFormat("Is Crouching: %s", isCrouching ? "true" : "false"), 575, 90, 20, PINK);
        DrawText(TextFormat("Animation Slice: %i %i", dinoAnimationIndexSlice[0], dinoAnimationIndexSlice[1]), 575, 130, 20, PINK);
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
void UpdateDinoAnimationIndexSlice(bool isCrouching, int dinoY, int dinoAnimationIndexSlice[])
{
    if (isCrouching)
    {
        dinoAnimationIndexSlice[0] = 0;
        dinoAnimationIndexSlice[1] = 0;
    }
    else if (dinoY < 280.0f)
    {
        dinoAnimationIndexSlice[0] = 0;
        dinoAnimationIndexSlice[1] = 0;
    }
    else
    {
        dinoAnimationIndexSlice[0] = 2;
        dinoAnimationIndexSlice[1] = 3;
    }
}

int UpdateFrameCounter(int frameCounter, int frameSpeed, int currentFrame, Rectangle frameRec)
{
    frameCounter++;
    if (frameCounter >= (60 / frameSpeed))
    {
        frameCounter = 0;
    }
    return frameCounter;
}

int UpdateCurrentFrame(int currentFrame, int frameCounter, int animationIndexSlice[])
{
    if (frameCounter == 0)
    {
        currentFrame++;
        if (currentFrame > animationIndexSlice[1] - animationIndexSlice[0])
            currentFrame = 0;
    }
    return currentFrame;
}

Rectangle UpdateFrameRec(int currentFrame, Rectangle frameRec, int animationIndexSlice[])
{
    frameRec.x = (float)currentFrame * (float)frameRec.width + (float)frameRec.width * animationIndexSlice[0];
    return frameRec;
}

int UpdateFrameSpeed(int frameSpeed)
{
    if (IsKeyPressed(KEY_RIGHT))
        frameSpeed++;
    else if (IsKeyPressed(KEY_LEFT))
        frameSpeed--;

    if (frameSpeed > MAX_FRAME_SPEED)
        frameSpeed = MAX_FRAME_SPEED;
    else if (frameSpeed < MIN_FRAME_SPEED)
        frameSpeed = MIN_FRAME_SPEED;

    return frameSpeed;
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

bool IsDucking()
{
    if (IsKeyDown(KEY_DOWN))
    {
        return true;
    }
    return false;
}
// ----------------------------------------------------------------------------------

// Collision Functions Definition
// ----------------------------------------------------------------------------------
bool IsColliding(Rectangle dinoRec, Rectangle cactusRec)
{
    if (CheckCollisionRecs(dinoRec, cactusRec))
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