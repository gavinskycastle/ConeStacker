#include "../libs/raylib/src/raylib.h"
#include "helper.hpp"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "../libs/raygui/src/raygui.h"

#include <vector>
#include <algorithm>
#include <string>
#include <iostream>

#include "leaderboard.hpp"

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
    bool enableTouchscreenControls = false;
};

enum GameState {
    PLAY_CLASSIC,
    PLAY_ARCADE,
    PLAY_RHYTHM,
    GAME_OVER,
    MAIN_MENU,
    PLAY_MENU,
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
    static Model trafficConeModel = LoadModel("../assets/trafficCone/tinker.obj");
    static Model yellowConeModel = LoadModel("../assets/yellowCone/tinker.obj");
    static Model orangeConeModel = LoadModel("../assets/orangeCone/tinker.obj");
    static Model blueConeModel = LoadModel("../assets/blueCone/tinker.obj");
    
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
const int screenWidth = 720;
const int screenHeight = 480;

Sound coneFall;
Sound coneDrop;

Image nateImage;
Image classicIcon;
Image arcadeIcon;
Image rhythmIcon;

Texture2D nateTexture;
Texture2D classicIconTexture;
Texture2D arcadeIconTexture;
Texture2D rhythmIconTexture;

// Define the camera to look into our 3d world
Camera3D camera = { 0 };

float targetFov = 45.0f;

bool windowShouldClose = false;

FloatingCone floatingCone;
GameSettings gameSettings;

GameState gameState = MAIN_MENU;
std::vector<Vector3> coneYs;

int score = 0;
bool highScoreEditMode = false;
char highScoreName[17] = "";

std::vector<std::string> leaderboardNames;
std::vector<int> leaderboardScores;
    
void init_app() {
    InitAudioDevice();
    coneFall = LoadSound("../assets/coneFall.ogg");
    coneDrop = LoadSound("../assets/coneDrop.ogg");
    
    camera.position = Vector3{ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    gameSettings.coneColor = CONE_TRAFFIC;
    
    nateImage = LoadImage("../assets/nate.png");
    
    classicIcon = LoadImage("../assets/icons/classic.png");
    //arcadeIcon = LoadImage("../assets/icons/arcade.png");
    //rhythmIcon = LoadImage("../assets/icons/rhythm.png");
    
    ImageResize(&nateImage, 576, 432);
    nateTexture = LoadTextureFromImage(nateImage);
    classicIconTexture = LoadTextureFromImage(classicIcon);
    //arcadeIconTexture = LoadTextureFromImage(arcadeIcon);
    //rhythmIconTexture = LoadTextureFromImage(rhythmIcon);
    
    ResetGame(coneYs, camera, targetFov, floatingCone);
    
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
}

bool app_loop() {
    // Update
    UpdateCamera(&camera, CAMERA_ORBITAL);
    
    // Calculate delta time in relation to 60 frames per second
    float relDt = GetFrameTime() * 60.0f;
    
    // Update controls
    switch(gameState) {
        case GAME_OVER: {
            floatingCone.fallSpeed += 0.01f * relDt; // Change by deltatime
            floatingCone.hoverDistance -= floatingCone.fallSpeed * relDt; // Change by deltatime
            camera.target = Vector3{floatingCone.x, coneYs.back().y + floatingCone.hoverDistance, 0.0f};
            break;
        }
        case PLAY_CLASSIC: {
            if ((IsKeyPressed(KEY_SPACE) && !gameSettings.enableTouchscreenControls) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && gameSettings.enableTouchscreenControls)) {
                if (floatingCone.x < 2.0f && floatingCone.x > -2.0f) {
                    coneYs.push_back(Vector3 {0.0f, coneYs.back().y + 1.0f, 0.0f});
                    
                    camera.target = coneYs.back();
                    if (targetFov < 90.0f) {
                        targetFov += 1.0f * relDt; // Change by deltatime
                    }
                    
                    floatingCone.x = 0.0f;
                    if (floatingCone.speed < 1.0f) {
                        floatingCone.speed += 0.025f;
                    }
                    PlaySound(coneDrop);
                } else {
                gameState = GAME_OVER;
                score = coneYs.size()-1;
                PlaySound(coneFall);
                }
            }
            if (floatingCone.toRight) {
                floatingCone.x += floatingCone.speed * relDt; // Change by deltatime
                if (floatingCone.x >= 4.0f) {
                    floatingCone.toRight = false;
                }
            } else {
                floatingCone.x -= floatingCone.speed * relDt; // Change by deltatime
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
            
            DrawCube(Vector3{1.5f, -5.0f, 1.5f}, 5.0f, 10.0f, 5.0f, GRAY);
            
            std::for_each (coneYs.begin(), coneYs.end(), [&](Vector3 coneY)
            {
                DrawCone(coneY, gameSettings.coneColor);
            });
            
            DrawCone(Vector3{floatingCone.x, coneYs.back().y + floatingCone.hoverDistance, 0.0f}, gameSettings.coneColor);

        EndMode3D();
        
        switch(gameState) {
            case GAME_OVER: {
                DrawTextCentered("Game Over", screenWidth/2, screenHeight/2-75, 50, BLACK);
                DrawTextCentered(("Score: " + std::to_string(score)).c_str(), screenWidth/2, screenHeight/2-25, 25, BLACK);
                //DrawTextCentered("Press Space To Play Again", screenWidth/2, screenHeight/2+75, 25, BLACK);
                if (checkScore(score)) {
                    DrawTextCentered("Enter your name to submit your high score", screenWidth/2, screenHeight/2+8, 16, BLACK);
                    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
                    if (GuiTextBox(Rectangle{screenWidth/2-60, screenHeight/2+32, 120, 24}, highScoreName, 16, highScoreEditMode)) {
                        highScoreEditMode = !highScoreEditMode;
                    }
                    GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
                    if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+75, 200, 50}, "Play Again") == 1) {
                        gameState = PLAY_CLASSIC;
                        ResetGame(coneYs, camera, targetFov, floatingCone);
                        saveScore(highScoreName, score);
                    };
                    if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+135, 200, 50}, "Main Menu") == 1) {
                        gameState = MAIN_MENU;
                        ResetGame(coneYs, camera, targetFov, floatingCone);
                        saveScore(highScoreName, score);
                    };
                } else {
                    GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
                    if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+25, 200, 50}, "Play Again") == 1) {
                        gameState = PLAY_CLASSIC;
                        ResetGame(coneYs, camera, targetFov, floatingCone);
                    };
                    if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+85, 200, 50}, "Main Menu") == 1) {
                        gameState = MAIN_MENU;
                        ResetGame(coneYs, camera, targetFov, floatingCone);
                    };
                }
                
                break;
            }
            case PLAY_CLASSIC: {
                DrawTextCentered(std::to_string(coneYs.size()-1).c_str(), screenWidth/2, 10, 50, BLACK);
                if (coneYs.size()-1 == 213) {
                    // Draw image of Nate
                    DrawTexture(nateTexture, screenWidth/2-nateImage.width/2, screenHeight/2-nateImage.height/2, WHITE);
                }
                break;
            }
            case PLAY_ARCADE: {
                break;
            }
            case PLAY_RHYTHM: {
                break;
            }
            case MAIN_MENU: {
                DrawTextCentered("Cone Stacker", screenWidth/2-35, screenHeight/2-125, 50, BLACK);
                DrawTextCentered("2", screenWidth/2+180, screenHeight/2-145, 75, BLACK);
                
                GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
                if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2-50, 200, 50}, "Play") == 1) {
                    gameState = PLAY_MENU;
                    ResetGame(coneYs, camera, targetFov, floatingCone);
                };
                if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+10, 200, 50}, "Leaderboard") == 1) {
                    gameState = LEADER_BOARD;
                    leaderboardNames = getNames();
                    if (leaderboardNames.size() > 10) leaderboardNames.erase(leaderboardNames.begin()+10, leaderboardNames.end());
                    leaderboardScores = getScores();
                    if (leaderboardScores.size() > 10) leaderboardScores.erase(leaderboardScores.begin()+10, leaderboardScores.end());
                };
                if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+70, 200, 50}, "Options") == 1) {
                    gameState = OPTIONS;
                };
                // Hide exit button on web
                #if !defined(PLATFORM_WEB)
                    if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+130, 200, 50}, "Exit") == 1) {
                        windowShouldClose = true;
                    };
                #endif
                
                DrawText("Made by Gavin P", 5, screenHeight-20, 15, BLACK);
                break;
            }
            case PLAY_MENU: {
                DrawTextCentered("Cone Stacker", screenWidth/2-35, screenHeight/2-125, 50, BLACK);
                DrawTextCentered("2", screenWidth/2+180, screenHeight/2-145, 75, BLACK);
                
                GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
                if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2-50, 200, 50}, "  Classic") == 1) {
                    gameState = PLAY_CLASSIC;
                    ResetGame(coneYs, camera, targetFov, floatingCone);
                };
                if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+10, 200, 50}, "  Arcade") == 1) {
                    gameState = PLAY_ARCADE;
                    ResetGame(coneYs, camera, targetFov, floatingCone);
                };
                if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+70, 200, 50}, "  Rhythm") == 1) {
                    gameState = PLAY_RHYTHM;
                    ResetGame(coneYs, camera, targetFov, floatingCone);
                };
                if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+130, 200, 50}, "Back") == 1) {
                    gameState = MAIN_MENU;
                };
                DrawTexture(classicIconTexture, screenWidth/2-100+25, screenHeight/2-50+7, WHITE);
                //DrawTexture(arcadeIconTexture, screenWidth/2-100, screenHeight/2+10, WHITE);
                //DrawTexture(rhythmIconTexture, screenWidth/2-100, screenHeight/2+70, WHITE);
                
                DrawText("Made by Gavin P", 5, screenHeight-20, 15, BLACK);
                break;
            }
            case OPTIONS: {
                GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
                
                GuiGroupBox(Rectangle{37, 50, 640, 400}, "Options");
                
                GuiLabel(Rectangle{62, 65, 120, 25}, "Cone Color");
                GuiToggleGroup(Rectangle{62, 90, 150, 25}, "Traffic;Yellow;Orange;Blue", &gameSettings.coneColor);
                
                GuiLabel(Rectangle {207, 170, 100, 25}, "SFX Volume");
                GuiSliderBar(Rectangle{302, 175, 120, 16}, NULL, NULL, &gameSettings.sfxVolume, 0, 1);
                GuiLabel(Rectangle{432, 170, 35, 25}, (std::to_string((int)(gameSettings.sfxVolume*100)) + "%").c_str());
                
                GuiCheckBox(Rectangle{62, 130, 25, 25}, "Enable Touchscreen Controls", &gameSettings.enableTouchscreenControls);
                
                SetSoundVolume(coneDrop, gameSettings.sfxVolume);
                SetSoundVolume(coneFall, gameSettings.sfxVolume);
                
                if (GuiButton(Rectangle{282, 410, 150, 25}, "Back to Main Menu") == 1) {
                    gameState = MAIN_MENU;
                }
                break;
            }
            case LEADER_BOARD: {
                GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
                
                GuiGroupBox(Rectangle{37, 50, 640, 400}, "Leaderboard");
                
                GuiLabel(Rectangle{92, 65, 120, 25}, "Name");
                
                DrawLine(87, 65, 87, 400, BLACK);
                DrawLine(540, 65, 540, 400, BLACK);
                
                GuiLabel(Rectangle{545, 65, 120, 25}, "Score");
                
                DrawLine(62, 95, 640, 95, BLACK);
                
                float yValue = 100.0f;
                for (std::string name : leaderboardNames) {
                    GuiLabel(Rectangle{92, yValue, 150, 25}, name.c_str());
                    yValue += 30.0f;
                }
                yValue = 100.0f;
                for (int score : leaderboardScores) {
                    GuiLabel(Rectangle{545, yValue, 150, 25}, std::to_string(score).c_str());
                    yValue += 30.0f;
                }
                
                if (GuiButton(Rectangle{282, 410, 150, 25}, "Back to Main Menu") == 1) {
                    gameState = MAIN_MENU;
                }
                break;
            }
        }
    EndDrawing();
    
    return !windowShouldClose;
}

void deinit_app() {
    UnloadSound(coneFall);
    UnloadSound(coneDrop);
    CloseAudioDevice();
}