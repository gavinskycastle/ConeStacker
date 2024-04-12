#include "../libs/raylib/src/raylib.h"
#include "helper.hpp"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "../libs/raygui/src/raygui.h"

#include <vector>
#include <algorithm>
#include <string>
#include <iostream>

struct FloatingCone {
    float x;
    float speed;
    float hoverDistance;
    float fallSpeed;
    bool toRight;
};

struct GameSettings {
    float sfxVolume = 1.0f;
    bool enableTouchscreenControls = false;
};

enum GameState {
    PLAY_CLASSIC,
    GAME_OVER
};

void DrawCone(Vector3 position) {
    static Model model = LoadModel("../assets/trafficCone/tinker.obj");
    
    DrawModelEx(model, position, Vector3{1.0f, 0.0f, 0.0f}, -90.0f, Vector3{0.2f, 0.2f, 0.2f}, WHITE); // Draw cone
}

void DrawTextCentered(const char* text, int posX, int posY, int fontSize, Color color) {
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, posX-(textWidth/2), posY, fontSize, color);
}

void ResetGame(std::vector<Vector3> &coneYs, Camera &camera, float &targetFov, FloatingCone &floatingCone) {
    coneYs.clear();
    coneYs.push_back(Vector3{0.0f, 0.0f, 0.0f});
    floatingCone.x = 0.0f;
    floatingCone.speed = 0.2f;
    floatingCone.fallSpeed = 0.0f;
    camera.fovy = 45.0f;
    camera.target = coneYs.back();
    targetFov = 45.0f;
    floatingCone.hoverDistance = 4.0f;
}

// Initialization
const int screenWidth = 1920;
const int screenHeight = 1080;

Sound coneFall;
Sound coneDrop;

// Define the camera to look into our 3d world
Camera3D camera = { 0 };

float targetFov = 45.0f;

bool windowShouldClose = false;

FloatingCone floatingCone;
GameSettings gameSettings;

GameState gameState = PLAY_CLASSIC;
std::vector<Vector3> coneYs;

int score = 0;
bool highScoreEditMode = false;
char highScoreName[17] = "";

int mouseTimer;
int oneTimeMouseTimer;
    
void init_app() {
    InitAudioDevice();
    coneFall = LoadSound("../assets/coneFall.ogg");
    coneDrop = LoadSound("../assets/coneDrop.ogg");
    
    camera.position = Vector3{ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
    
    ResetGame(coneYs, camera, targetFov, floatingCone);
    
    mouseTimer = 0;
    oneTimeMouseTimer = 0;
    
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    ToggleFullscreen();
}

bool app_loop() {
    // Update
    UpdateCamera(&camera, CAMERA_ORBITAL);
    
    if (mouseTimer < 60) {
        mouseTimer++;
    }
    if (oneTimeMouseTimer < 30) {
        oneTimeMouseTimer++;
    }
    
    // Update controls
    switch(gameState) {
        case GAME_OVER: {
            floatingCone.x = 4.0f;
            floatingCone.fallSpeed += 0.01f;
            floatingCone.hoverDistance -= floatingCone.fallSpeed;
            camera.target = Vector3{floatingCone.x, coneYs.back().y + floatingCone.hoverDistance, 0.0f};
            if (mouseTimer == 60) {
                gameState = PLAY_CLASSIC;
                mouseTimer = 0;
                ResetGame(coneYs, camera, targetFov, floatingCone);
            }
            break;
        }
        case PLAY_CLASSIC: {
            // Generate a boolean that will only be true 1/60th of the time
            bool shouldDropCone = GetRandomValue(0, 60) == 0;
            if (shouldDropCone) {
                if (floatingCone.x < 2.0f && floatingCone.x > -2.0f) {
                    floatingCone.x = 4.0f;
                    if (floatingCone.speed < 1.0f) {
                        floatingCone.speed += 0.025f;
                        PlaySound(coneDrop);
                    }
                    gameState = GAME_OVER;
                    mouseTimer = 0;
                    score = coneYs.size()-1;
                    PlaySound(coneFall);
                } else {
                    coneYs.push_back(Vector3 {0.0f, coneYs.back().y + 1.0f, 0.0f});
                    
                    camera.target = coneYs.back();
                    if (targetFov < 90.0f) {
                        targetFov += 1.0f;
                    }
                }
            }
            
            if (floatingCone.toRight) {
                floatingCone.x += floatingCone.speed;
                if (floatingCone.x >= 4.0f) {
                    floatingCone.toRight = false;
                }
            } else {
                floatingCone.x -= floatingCone.speed;
                if (floatingCone.x <= -4.0f) {
                    floatingCone.toRight = true;
                }
            }
        }
        default: {
            break;
        }
    }
    
    if (targetFov > camera.fovy) {
        camera.fovy += 0.1f;
    } else if (targetFov < camera.fovy) {
        camera.fovy -= 0.1f;
    }
    // Draw
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        BeginMode3D(camera);
            
            DrawCube(Vector3{1.5f, -5.0f, 1.5f}, 5.0f, 10.0f, 5.0f, GRAY);
            
            std::for_each (coneYs.begin(), coneYs.end(), [&](Vector3 coneY)
            {
                DrawCone(coneY);
            });
            
            DrawCone(Vector3{floatingCone.x, coneYs.back().y + floatingCone.hoverDistance, 0.0f});

        EndMode3D();
        
        if (gameState == PLAY_CLASSIC) {
            DrawTextCentered(std::to_string(coneYs.size()-1).c_str(), screenWidth/2, 10, 50, BLACK);
        }
    EndDrawing();
    
    // Exit the window if any mouse click or mouse movement is detected
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) || IsMouseButtonDown(MOUSE_LEFT_BUTTON) || IsMouseButtonDown(MOUSE_RIGHT_BUTTON) || IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) || IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || IsMouseButtonReleased(MOUSE_RIGHT_BUTTON) || IsMouseButtonReleased(MOUSE_MIDDLE_BUTTON) || (GetMouseDelta().x != 0 && mouseTimer > 30) || (GetMouseDelta().y != 0 && mouseTimer > 30)) {
        windowShouldClose = true;
    }
    
    return !windowShouldClose;
}

void deinit_app() {
    UnloadSound(coneFall);
    UnloadSound(coneDrop);
    CloseAudioDevice();
}