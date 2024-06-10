#include "../libs/raylib/src/raylib.h"
#include "helper.hpp"

#define RAYGUI_IMPLEMENTATION
#include "../libs/raygui/src/raygui.h"

#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <cmath>

#include "main.hpp"
#include "leaderboard.hpp"

struct FloatingCone {
    float x;
    float speed;
    float hoverDistance;
    float fallSpeed;
    bool toRight;
};

struct ArcadeMessage {
    std::string message;
    float time;
    bool gold;
};

enum ConeColor {
    CONE_TRAFFIC,
    CONE_YELLOW,
    CONE_ORANGE,
    CONE_BLUE,
    ARCADE_CONE_GOLD,
    ARCADE_CONE_RED,
};

struct GameSettings {
    ConeColor coneColor;
    int coneColorIndex = 0;
    float sfxVolume = 1.0f;
    bool enableTouchscreenControls = false;
};

void DrawCone(Vector3 position, int color) {
    static Model trafficConeModel = LoadModel("../assets/trafficCone/tinker.obj");
    static Model yellowConeModel = LoadModel("../assets/yellowCone/tinker.obj");
    static Model orangeConeModel = LoadModel("../assets/orangeCone/tinker.obj");
    static Model blueConeModel = LoadModel("../assets/blueCone/tinker.obj");
    
    static Model arcadeGoldConeModel = LoadModel("../assets/goldCone/tinker.obj");
    static Model arcadeRedConeModel = LoadModel("../assets/redCone/tinker.obj");
    
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
        case ARCADE_CONE_GOLD:
            model = arcadeGoldConeModel;
            break;
        case ARCADE_CONE_RED:
            model = arcadeRedConeModel;
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

void ResetGame(std::vector<Vector3> &coneYs, Camera &camera, float &targetFov, FloatingCone &floatingCone, std::vector<ConeColor> &arcadeConeColors, ConeColor &arcadeConeColor, int &score, std::vector<ArcadeMessage> &arcadeMessages) {
    score = 0;
    coneYs.clear();
    coneYs.push_back(Vector3{0.0f, 0.0f, 0.0f});
    floatingCone.x = 0.0f;
    floatingCone.speed = 0.2f;
    floatingCone.fallSpeed = 0.0f;
    camera.fovy = 45.0f;
    camera.target = coneYs.back();
    targetFov = 45.0f;
    floatingCone.hoverDistance = 4.0f;
    arcadeConeColors.clear();
    arcadeMessages.clear();
    arcadeConeColors.push_back(CONE_TRAFFIC);
    arcadeConeColor = CONE_TRAFFIC;
}

void addNewArcadeCone(std::vector<Vector3> &coneYs, std::vector<ConeColor> &arcadeConeColors, ConeColor &arcadeConeColor, float floatingConeX, int &score, std::vector<ArcadeMessage> &arcadeMessages) {
    ConeColor prevArcadeConeColor = arcadeConeColor;
    
    arcadeConeColors.push_back(arcadeConeColor);
    coneYs.push_back(Vector3{0.0f, coneYs.back().y + 1.0f, 0.0f});
    
    int randomNumber = rand() % 100;
    
    if (randomNumber < 23) {
        arcadeConeColor = CONE_TRAFFIC;
    } else if (randomNumber < 46) {
        arcadeConeColor = CONE_YELLOW;
    } else if (randomNumber < 69) {
        arcadeConeColor = CONE_ORANGE;
    } else if (randomNumber < 92) {
        arcadeConeColor = CONE_BLUE;
    } else if (randomNumber < 96) {
        arcadeConeColor = ARCADE_CONE_GOLD;
    } else {
        arcadeConeColor = ARCADE_CONE_RED;
    }
    
    float deltaScore = (2.0 - fabs(floatingConeX)) * 50.0f;
    
    ArcadeMessage message;
    
    if (deltaScore < 20) {
        message = ArcadeMessage{"Good!", 1.0f, false};
    } else if (deltaScore < 60) {
        message = ArcadeMessage{"Great!", 1.0f, false};
    } else if (deltaScore < 90) {
        message = ArcadeMessage{"Nice!", 1.0f, false};
    } else if (deltaScore < 95) {
        message = ArcadeMessage{"Outstanding!", 1.0f, false};
    } else if (deltaScore < 100) {
        message = ArcadeMessage{"Perfect!", 1.0f, false};
    }
    
    if (prevArcadeConeColor == ARCADE_CONE_GOLD) {
        message.gold = true;
        deltaScore *= 10;
    }
    
    arcadeMessages.push_back(message);
    score += deltaScore;
}

void addNewClassicCone(std::vector<Vector3> &coneYs, std::vector<ConeColor> &arcadeConeColors, ConeColor &arcadeConeColor, GameSettings &gameSettings) {
    arcadeConeColors.push_back(gameSettings.coneColor);
    coneYs.push_back(Vector3 {0.0f, coneYs.back().y + 1.0f, 0.0f});
    
    arcadeConeColor = gameSettings.coneColor;
}

void loadLeaderboardData(std::vector<std::string> &leaderboardNames, std::vector<int> &leaderboardScores) {
    leaderboardNames = getNames();
    if (leaderboardNames.size() > 10) leaderboardNames.erase(leaderboardNames.begin()+10, leaderboardNames.end());
    leaderboardScores = getScores();
    if (leaderboardScores.size() > 10) leaderboardScores.erase(leaderboardScores.begin()+10, leaderboardScores.end());
}

// Initialization
const int screenWidth = 720;
const int screenHeight = 480;

Sound coneFall;
Sound coneDrop;
Sound goldCone;

Image nateImage;
Image classicIcon;
Image arcadeIcon;
Image duelsIcon;

Texture2D nateTexture;
Texture2D classicIconTexture;
Texture2D arcadeIconTexture;
Texture2D duelsIconTexture;

// Define the camera to look into our 3d world
Camera3D camera = { 0 };

float targetFov = 45.0f;

bool windowShouldClose = false;

FloatingCone floatingCone;
GameSettings gameSettings;

GameState gameState = MAIN_MENU;
GameState gameOverReturnState = PLAY_CLASSIC;
std::vector<Vector3> coneYs;
std::vector<ConeColor> coneColors = {CONE_TRAFFIC};
std::vector<ArcadeMessage> arcadeMessages;
ConeColor coneColor = CONE_TRAFFIC;

int score = 0;
bool highScoreEditMode = false;
char highScoreName[17] = "";

std::vector<std::string> leaderboardNames;
std::vector<int> leaderboardScores;

GameState selectedLeaderboardMode;
int selectedLeaderboardIndex = 0;

void UpdateGameOver(float relDt) {
    floatingCone.fallSpeed += 0.01f * relDt; // Change by deltatime
    floatingCone.hoverDistance -= floatingCone.fallSpeed * relDt; // Change by deltatime
    camera.target = Vector3{floatingCone.x, coneYs.back().y + floatingCone.hoverDistance, 0.0f};
}

void UpdatePlayClassic(float relDt) {
    score = coneYs.size()-1;
    if ((IsKeyPressed(KEY_SPACE) && !gameSettings.enableTouchscreenControls) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && gameSettings.enableTouchscreenControls)) {
        if (floatingCone.x < 2.0f && floatingCone.x > -2.0f) {
            addNewClassicCone(coneYs, coneColors, coneColor, gameSettings);
            
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
}

void UpdatePlayArcade(float relDt) {
    if ((IsKeyPressed(KEY_SPACE) && !gameSettings.enableTouchscreenControls) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && gameSettings.enableTouchscreenControls)) {
        if (floatingCone.x < 2.0f && floatingCone.x > -2.0f) {
            if (coneColor == ARCADE_CONE_GOLD) PlaySound(goldCone);
            addNewArcadeCone(coneYs, coneColors, coneColor, floatingCone.x, score, arcadeMessages);
            
            camera.target = coneYs.back();
            if (targetFov < 90.0f) {
                targetFov += 1.0f * relDt; // Change by deltatime
            }
            
            floatingCone.x = 0.0f;
            if (floatingCone.speed < 0.5f) { // Lowered max speed for arcade mode
                floatingCone.speed += 0.025f;
            }
            PlaySound(coneDrop);
        } else {
        gameState = GAME_OVER;
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
}

void UpdateCameraFOV() {
    if (targetFov > camera.fovy) {
        camera.fovy += 0.1f;
    } else if (targetFov < camera.fovy) {
        camera.fovy -= 0.1f;
    }
}

void DrawConeStack() {
    DrawCube(Vector3{1.5f, -5.0f, 1.5f}, 5.0f, 10.0f, 5.0f, GRAY);
            
    int i = 0;
    std::for_each (coneYs.begin(), coneYs.end(), [&](Vector3 coneY)
    {
        DrawCone(coneY, (gameState == PLAY_ARCADE || gameOverReturnState == PLAY_ARCADE) ? coneColors.at(i) : gameSettings.coneColor);
        i++;
    });
    
    DrawCone(Vector3{floatingCone.x, coneYs.back().y + floatingCone.hoverDistance, 0.0f}, (gameState == PLAY_ARCADE || gameOverReturnState == PLAY_ARCADE) ? coneColor : gameSettings.coneColor);
}

void DrawGameOver() {
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
            gameState = gameOverReturnState;
            selectLeaderboardMode(gameOverReturnState);
            saveScore(highScoreName, score);
            ResetGame(coneYs, camera, targetFov, floatingCone, coneColors, coneColor, score, arcadeMessages);
        };
        if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+135, 200, 50}, "Main Menu") == 1) {
            gameState = MAIN_MENU;
            selectLeaderboardMode(gameOverReturnState);
            saveScore(highScoreName, score);
            ResetGame(coneYs, camera, targetFov, floatingCone, coneColors, coneColor, score, arcadeMessages);
        };
    } else {
        GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
        if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+25, 200, 50}, "Play Again") == 1) {
            gameState = gameOverReturnState;
            ResetGame(coneYs, camera, targetFov, floatingCone, coneColors, coneColor, score, arcadeMessages);
        };
        if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+85, 200, 50}, "Main Menu") == 1) {
            gameState = MAIN_MENU;
            ResetGame(coneYs, camera, targetFov, floatingCone, coneColors, coneColor, score, arcadeMessages);
        };
    }
}

void DrawClassicScore() {
    DrawTextCentered(std::to_string(score).c_str(), screenWidth/2, 10, 50, BLACK);
    if (coneYs.size()-1 == 213) {
        // Draw image of Nate
        DrawTexture(nateTexture, screenWidth/2-nateImage.width/2, screenHeight/2-nateImage.height/2, WHITE);
    }
}

void DrawArcadeScore(float relDt) {
    DrawTextCentered(std::to_string(score).c_str(), screenWidth/2, 10, 50, BLACK);
    // Loop through arcadeMessages and reduce their time by deltatime
    for (int i = 0; i < arcadeMessages.size(); i++) {
        arcadeMessages.at(i).time -= (relDt / 60.0f);
    }
    // Draw arcadeMessages
    for (int i = 0; i < arcadeMessages.size(); i++) {
        Color messageColor;
        if (arcadeMessages.at(i).gold) {
            messageColor = Color{211, 176, 56, (unsigned char)(arcadeMessages.at(i).time * 255)};
        } else {
            messageColor = Color{0, 0, 0, (unsigned char)(arcadeMessages.at(i).time * 255)};
        }
        DrawTextCentered(arcadeMessages.at(i).message.c_str(), screenWidth/2, screenHeight/2-210+(arcadeMessages.at(i).time * 50), 25, messageColor);
    }
    for (int i = 0; i < arcadeMessages.size(); i++) {
        if (arcadeMessages.at(i).time <= 0.0f) {
            arcadeMessages.erase(arcadeMessages.begin()+i);
        }
    }
}

void DrawMainMenu() {
    DrawTextCentered("Cone Stacker", screenWidth/2-35, screenHeight/2-125, 50, BLACK);
    DrawTextCentered("2", screenWidth/2+180, screenHeight/2-145, 75, BLACK);
    
    GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
    if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2-50, 200, 50}, "Play") == 1) {
        gameState = PLAY_MENU;
        ResetGame(coneYs, camera, targetFov, floatingCone, coneColors, coneColor, score, arcadeMessages);
    };
    if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+10, 200, 50}, "Leaderboard") == 1) {
        gameState = LEADER_BOARD;
        selectLeaderboardMode(PLAY_CLASSIC);
        selectedLeaderboardMode = PLAY_CLASSIC;
        loadLeaderboardData(leaderboardNames, leaderboardScores);
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
    
    if (GuiButton(Rectangle {32-2, screenHeight-100-1, 52, 52}, "") == 1) {
        // Profile button hit
    }
    GuiDrawIcon(ICON_PLAYER, 32, screenHeight-100, 3, GRAY);
    if (GuiButton(Rectangle {102-2, screenHeight-100-1, 52, 52}, "") == 1) {
        // Messages button hit
    }
    GuiDrawIcon(ICON_MAILBOX, 102, screenHeight-100, 3, GRAY);
}

void DrawPlayMenu() {
    DrawTextCentered("Cone Stacker", screenWidth/2-35, screenHeight/2-125, 50, BLACK);
    DrawTextCentered("2", screenWidth/2+180, screenHeight/2-145, 75, BLACK);
    
    GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
    if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2-50, 200, 50}, "  Classic") == 1) {
        gameState = PLAY_CLASSIC;
        gameOverReturnState = PLAY_CLASSIC;
        ResetGame(coneYs, camera, targetFov, floatingCone, coneColors, coneColor, score, arcadeMessages);
    };
    if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+10, 200, 50}, "  Arcade") == 1) {
        gameState = PLAY_ARCADE;
        gameOverReturnState = PLAY_ARCADE;
        ResetGame(coneYs, camera, targetFov, floatingCone, coneColors, coneColor, score, arcadeMessages);
    };
    if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+70, 200, 50}, "  Duels") == 1) {
        gameState = PLAY_DUELS;
        gameOverReturnState = PLAY_DUELS;
        ResetGame(coneYs, camera, targetFov, floatingCone, coneColors, coneColor, score, arcadeMessages);
    };
    if (GuiButton(Rectangle {screenWidth/2-100, screenHeight/2+130, 200, 50}, "Back") == 1) {
        gameState = MAIN_MENU;
    };
    DrawTexture(classicIconTexture, screenWidth/2-100+25, screenHeight/2-43, WHITE);
    DrawTexture(arcadeIconTexture, screenWidth/2-100+20, screenHeight/2+17, WHITE);
    DrawTexture(duelsIconTexture, screenWidth/2-100+20, screenHeight/2+77, WHITE);
}

void DrawOptions() {
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
                
    GuiGroupBox(Rectangle{37, 50, 640, 400}, "Options");
    
    GuiLabel(Rectangle{62, 65, 150, 25}, "Classic Cone Color");
    gameSettings.coneColorIndex = static_cast<int>(gameSettings.coneColor);
    GuiToggleGroup(Rectangle{62, 90, 150, 25}, "Traffic;Yellow;Orange;Blue", &gameSettings.coneColorIndex);
    gameSettings.coneColor = static_cast<ConeColor>(gameSettings.coneColorIndex);
    
    GuiLabel(Rectangle {207, 170, 100, 25}, "SFX Volume");
    GuiSliderBar(Rectangle{302, 175, 120, 16}, NULL, NULL, &gameSettings.sfxVolume, 0, 1);
    GuiLabel(Rectangle{432, 170, 35, 25}, (std::to_string((int)(gameSettings.sfxVolume*100)) + "%").c_str());
    
    GuiCheckBox(Rectangle{62, 130, 25, 25}, "Enable Touchscreen Controls", &gameSettings.enableTouchscreenControls);
    
    SetSoundVolume(coneDrop, gameSettings.sfxVolume);
    SetSoundVolume(coneFall, gameSettings.sfxVolume);
    
    if (GuiButton(Rectangle{282, 410, 150, 25}, "Back to Main Menu") == 1) {
        gameState = MAIN_MENU;
    }
}

void DrawLeaderboard() {
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
    
    GuiGroupBox(Rectangle{37, 20, 640, 430}, "Leaderboard");
    
    selectedLeaderboardIndex = static_cast<int>(selectedLeaderboardMode);
    GuiToggleGroup(Rectangle{55, 35, 150, 25}, "Classic;Arcade", &selectedLeaderboardIndex);
    if (static_cast<GameState>(selectedLeaderboardIndex) != selectedLeaderboardMode) {
        selectedLeaderboardMode = static_cast<GameState>(selectedLeaderboardIndex);
        selectLeaderboardMode(selectedLeaderboardMode);
        loadLeaderboardData(leaderboardNames, leaderboardScores);
    }
    
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
}

void init_app() {
    InitAudioDevice();
    coneFall = LoadSound("../assets/coneFall.ogg");
    coneDrop = LoadSound("../assets/coneDrop.ogg");
    goldCone = LoadSound("../assets/gold.wav");
    
    camera.position = Vector3{ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                              // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;           // Camera projection type

    gameSettings.coneColor = CONE_TRAFFIC;
    
    nateImage = LoadImage("../assets/nate.png");
    
    classicIcon = LoadImage("../assets/icons/classic.png");
    arcadeIcon = LoadImage("../assets/icons/arcade.png");
    duelsIcon = LoadImage("../assets/icons/duels.png");
    
    ImageResize(&nateImage, 576, 432);
    nateTexture = LoadTextureFromImage(nateImage);
    classicIconTexture = LoadTextureFromImage(classicIcon);
    arcadeIconTexture = LoadTextureFromImage(arcadeIcon);
    duelsIconTexture = LoadTextureFromImage(duelsIcon);
    
    ResetGame(coneYs, camera, targetFov, floatingCone, coneColors, coneColor, score, arcadeMessages);
    
    selectLeaderboardMode(PLAY_CLASSIC);
}

bool app_loop() {
    UpdateCamera(&camera, CAMERA_ORBITAL);
    
    float relDt = GetFrameTime() * 60.0f; // Calculate delta time in relation to 60 frames per second
    
    // Update controls
    switch(gameState) {
        case GAME_OVER: {UpdateGameOver(relDt); break;}
        case PLAY_CLASSIC: {UpdatePlayClassic(relDt); break;}
        case PLAY_ARCADE: {UpdatePlayArcade(relDt); break;}
        default: {break;}
    }
    
    UpdateCameraFOV();
    
    // Draw
    BeginDrawing();
        ClearBackground(LIGHTGRAY);
        
        // 3D rendering
        BeginMode3D(camera);
            DrawConeStack();
        EndMode3D();
        
        // 2D rendering
        switch(gameState) {
            case GAME_OVER: {DrawGameOver(); break;}
            case PLAY_CLASSIC: {DrawClassicScore(); break;}
            case PLAY_ARCADE: {DrawArcadeScore(relDt); break;}
            case PLAY_DUELS: {break;}
            case MAIN_MENU: {DrawMainMenu(); break;}
            case PLAY_MENU: {DrawPlayMenu(); break;}
            case OPTIONS: {DrawOptions(); break;}
            case LEADER_BOARD: {DrawLeaderboard(); break;}
            default: {break;}
        }
    EndDrawing();
    
    return !windowShouldClose;
}

void deinit_app() {
    UnloadSound(coneFall);
    UnloadSound(coneDrop);
    CloseAudioDevice();
}