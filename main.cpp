#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <vector>
#include <algorithm>
#include <string>
#include <iostream>

#include "leaderboard.h"

struct FloatingCone {
    float x;
    float speed;
    float hoverDistance;
    float fallSpeed;
    bool toRight;
};

struct GameSettings {
    int coneColor;
    float sfxVolume = 1.0f;
};

enum GameState {
    PLAY,
    GAME_OVER,
    MAIN_MENU,
    OPTIONS,
    LEADER_BOARD,
};

enum ConeColor {
    CONE_TRAFFIC,
    CONE_YELLOW,
    CONE_ORANGE,
    CONE_BLUE
};


void DrawCone(Vector3 position, int color) {
    static Model trafficConeModel = LoadModel("resources/trafficCone/tinker.obj");
    static Model yellowConeModel = LoadModel("resources/yellowCone/tinker.obj");
    static Model orangeConeModel = LoadModel("resources/orangeCone/tinker.obj");
    static Model blueConeModel = LoadModel("resources/blueCone/tinker.obj");
    
    Model model;
    
    switch(color) {
        case CONE_YELLOW:
            model = yellowConeModel;
            break;
        case CONE_ORANGE:
            model = orangeConeModel;
            break;
        case CONE_BLUE:
            model = blueConeModel;
            break;
        default:
        case CONE_TRAFFIC:
            model = trafficConeModel;
            break;
    }
    
    DrawModelEx(model, position, (Vector3){1.0f, 0.0f, 0.0f}, -90.0f, (Vector3){0.2f, 0.2f, 0.2f}, WHITE); // Draw cone
}

void DrawTextCentered(const char* text, int posX, int posY, int fontSize, Color color) {
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, posX-(textWidth/2), posY, fontSize, color);
}

void ResetGame(std::vector<Vector3> &coneYs, Camera &camera, float &targetFov, FloatingCone &floatingCone) {
    coneYs.clear();
    coneYs.push_back((Vector3){0.0f, 0.0f, 0.0f});
    floatingCone.x = 0.0f;
    floatingCone.speed = 0.2f;
    floatingCone.fallSpeed = 0.0f;
    camera.fovy = 45.0f;
    camera.target = coneYs.back();
    targetFov = 45.0f;
    floatingCone.hoverDistance = 4.0f;
}

// Program main entry point
int main(void)
{
    // Initialization
    const int screenWidth = 720;
    const int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "Cone Stacker");
    Image windowIcon = LoadImage("resources/icon.png");
    SetWindowIcon(windowIcon);
    
    InitAudioDevice();
    Sound coneFall = LoadSound("resources/coneFall.ogg");
    Sound coneDrop = LoadSound("resources/coneDrop.ogg");
    
    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
    
    float targetFov = 45.0f;
    
    bool windowShouldClose = false;
    
    FloatingCone floatingCone;
    GameSettings gameSettings;
    gameSettings.coneColor = CONE_TRAFFIC;
    GameState gameState = MAIN_MENU;
    std::vector<Vector3> coneYs;
    
    ResetGame(coneYs, camera, targetFov, floatingCone);
    
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    
    // Main game loop
    while (!WindowShouldClose() && !windowShouldClose)        // Detect window close button or ESC key
    {
        // Update
        UpdateCamera(&camera, CAMERA_ORBITAL);
        
        // Update controls
        switch(gameState) {
            case GAME_OVER: {
                floatingCone.fallSpeed += 0.01f;
                floatingCone.hoverDistance -= floatingCone.fallSpeed;
                camera.target = (Vector3){floatingCone.x, coneYs.back().y + floatingCone.hoverDistance, 0.0f};
                break;
            }
            case PLAY: {
                if (IsKeyPressed(KEY_SPACE)) {
                    if (floatingCone.x < 2.0f && floatingCone.x > -2.0f) {
                        coneYs.push_back((Vector3) {0.0f, coneYs.back().y + 1.0f, 0.0f});
                        
                        camera.target = coneYs.back();
                        if (targetFov < 90.0f) {
                            targetFov += 1.0f;
                        }
                        
                        floatingCone.x = 0.0f;
                        if (floatingCone.speed < 1.0f) {
                            floatingCone.speed += 0.025f;
                        }
                        PlaySound(coneDrop);
                    } else {
                    gameState = GAME_OVER;
                    PlaySound(coneFall);
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
                break;
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
                
                DrawCube((Vector3){1.5f, -5.0f, 1.5f}, 5.0f, 10.0f, 5.0f, GRAY);
                
                std::for_each (coneYs.begin(), coneYs.end(), [&](Vector3 coneY)
                {
                    DrawCone(coneY, gameSettings.coneColor);
                });
                
                DrawCone((Vector3){floatingCone.x, coneYs.back().y + floatingCone.hoverDistance, 0.0f}, gameSettings.coneColor);

            EndMode3D();
            
            switch(gameState) {
                case GAME_OVER: {
                    DrawTextCentered("Game Over", screenWidth/2, screenHeight/2-75, 50, BLACK);
                    DrawTextCentered(("Score: " + std::to_string(coneYs.size()-1)).c_str(), screenWidth/2, screenHeight/2-25, 25, BLACK);
                    //DrawTextCentered("Press Space To Play Again", screenWidth/2, screenHeight/2+75, 25, BLACK);
                    GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
                    if (GuiButton((Rectangle) {screenWidth/2-100, screenHeight/2+25, 200, 50}, "Play Again") == 1) {
                        gameState = PLAY;
                        ResetGame(coneYs, camera, targetFov, floatingCone);
                    };
                    if (GuiButton((Rectangle) {screenWidth/2-100, screenHeight/2+85, 200, 50}, "Main Menu") == 1) {
                        gameState = MAIN_MENU;
                        ResetGame(coneYs, camera, targetFov, floatingCone);
                    };
                    
                    break;
                }
                case PLAY: {
                    DrawTextCentered(std::to_string(coneYs.size()-1).c_str(), screenWidth/2, 10, 50, BLACK);
                    break;
                }
                case MAIN_MENU: {
                    DrawTextCentered("Cone Stacker", screenWidth/2, screenHeight/2-125, 50, BLACK);
                    
                    GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
                    if (GuiButton((Rectangle) {screenWidth/2-100, screenHeight/2-50, 200, 50}, "Play") == 1) {
                        gameState = PLAY;
                        ResetGame(coneYs, camera, targetFov, floatingCone);
                    };
                    if (GuiButton((Rectangle) {screenWidth/2-100, screenHeight/2+10, 200, 50}, "Leaderboard") == 1) {
                        gameState = LEADER_BOARD;
                    };
                    if (GuiButton((Rectangle) {screenWidth/2-100, screenHeight/2+70, 200, 50}, "Options") == 1) {
                        gameState = OPTIONS;
                    };
                    if (GuiButton((Rectangle) {screenWidth/2-100, screenHeight/2+130, 200, 50}, "Exit") == 1) {
                        windowShouldClose = true;
                    };
                    
                    DrawText("Made by Gavin P", 5, screenHeight-20, 15, BLACK);
                    break;
                }
                case OPTIONS: {
                    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
                    
                    GuiGroupBox((Rectangle){37, 50, 640, 400}, "Options");
                    
                    GuiLabel((Rectangle){62, 65, 120, 25}, "Cone Color");
                    GuiToggleGroup((Rectangle){62, 90, 150, 25}, "Traffic;Yellow;Orange;Blue", &gameSettings.coneColor);
                    
                    GuiLabel((Rectangle) {207, 170, 100, 25}, "SFX Volume");
                    GuiSliderBar((Rectangle){302, 175, 120, 16}, NULL, NULL, &gameSettings.sfxVolume, 0, 1);
                    GuiLabel((Rectangle){432, 170, 35, 25}, (std::to_string((int)(gameSettings.sfxVolume*100)) + "%").c_str());
                    
                    SetSoundVolume(coneDrop, gameSettings.sfxVolume);
                    SetSoundVolume(coneFall, gameSettings.sfxVolume);
                    
                    if (GuiButton((Rectangle){282, 410, 150, 25}, "Back to Main Menu") == 1) {
                        gameState = MAIN_MENU;
                    }
                    break;
                }
                case LEADER_BOARD: {
                    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
                    if (GuiButton((Rectangle){282, 410, 150, 25}, "Back to Main Menu") == 1) {
                        gameState = MAIN_MENU;
                    }
                    break;
                }
            }
        EndDrawing();
        
    }

    // De-Initialization
    CloseWindow();        // Close window and OpenGL context

    return 0;
}