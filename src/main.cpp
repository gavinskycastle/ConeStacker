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
#include <raymath.h>

// Window setup
int screenWidth = 720;
const int screenHeight = 480;
bool windowShouldClose = false;

// Loading resources
Sound coneFall;
Sound coneDrop;
Sound goldCone;
Sound redCone;
Image nateImage;
Image classicIcon;
Image arcadeIcon;
Image duelsIcon;
Texture2D nateTexture;
Texture2D classicIconTexture;
Texture2D arcadeIconTexture;
Texture2D duelsIconTexture;

// Leaderboard/name entry setup
bool highScoreEditMode = false;
char highScoreName[17] = "";
std::vector<std::string> leaderboardNames;
std::vector<int> leaderboardScores;
GameState selectedLeaderboardMode;
int selectedLeaderboardIndex = 0;

// Global game state setup
GameSettings gameSettings;

RenderTexture2D mainRenderTexture;
RenderTexture2D player1RenderTexture;
RenderTexture2D player2RenderTexture;
GameInstanceState mainGameInstance;
GameInstanceState player2GameInstance;

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

void ResetGame(GameInstanceState &gameInstance) {
    gameInstance.score = 0;
    gameInstance.coneYs.clear();
    gameInstance.coneYs.push_back(Vector3{0.0f, 0.0f, 0.0f});
    gameInstance.floatingCone.x = 0.0f;
    gameInstance.floatingCone.speed = 0.2f;
    gameInstance.floatingCone.fallSpeed = 0.0f;
    gameInstance.camera.fovy = 45.0f;
    gameInstance.camera.target = gameInstance.coneYs.back();
    gameInstance.targetFov = 45.0f;
    gameInstance.floatingCone.hoverDistance = 4.0f;
    gameInstance.coneColors.clear();
    gameInstance.arcadeMessages.clear();
    gameInstance.coneColors.push_back(CONE_TRAFFIC);
    gameInstance.coneColor = CONE_TRAFFIC;
}

void InitGameInstance(GameInstanceState &gameInstance) {
    gameInstance.camera.position = Vector3{ 10.0f, 10.0f, 10.0f }; // Camera position
    gameInstance.camera.target = Vector3{ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    gameInstance.camera.up = Vector3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    gameInstance.camera.fovy = 45.0f;                              // Camera field-of-view Y
    gameInstance.camera.projection = CAMERA_PERSPECTIVE;           // Camera projection type
    ResetGame(gameInstance);
}

ConeColor randomArcadeConeColor() {
    ConeColor arcadeConeColor;
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
    
    return arcadeConeColor;
}

void addNewArcadeCone(GameInstanceState &gameInstance) {
    ConeColor prevArcadeConeColor = gameInstance.coneColor;
    
    gameInstance.coneColors.push_back(gameInstance.coneColor);
    gameInstance.coneYs.push_back(Vector3{0.0f, gameInstance.coneYs.back().y + 1.0f, 0.0f});
    
    gameInstance.coneColor = randomArcadeConeColor();
    
    float deltaScore = (2.0 - fabs(gameInstance.floatingCone.x)) * 50.0f;
    
    ArcadeMessage message;
    
    if (deltaScore < 20) {
        message = ArcadeMessage{"Good!", 1.0f};
    } else if (deltaScore < 60) {
        message = ArcadeMessage{"Great!", 1.0f};
    } else if (deltaScore < 90) {
        message = ArcadeMessage{"Nice!", 1.0f};
    } else if (deltaScore < 95) {
        message = ArcadeMessage{"Outstanding!", 1.0f};
    } else if (deltaScore < 100) {
        message = ArcadeMessage{"Perfect!", 1.0f};
    }
    
    if (prevArcadeConeColor == ARCADE_CONE_GOLD) {
        message.gold = true;
        deltaScore *= 10;
    }
    
    gameInstance.arcadeMessages.push_back(message);
    gameInstance.score += deltaScore;
}

void addNewClassicCone(GameInstanceState &gameInstance, GameSettings &gameSettings) {
    gameInstance.coneColors.push_back(gameSettings.coneColor);
    gameInstance.coneYs.push_back(Vector3 {0.0f, gameInstance.coneYs.back().y + 1.0f, 0.0f});
    
    gameInstance.coneColor = gameSettings.coneColor;
}

void loadLeaderboardData(std::vector<std::string> &leaderboardNames, std::vector<int> &leaderboardScores) {
    leaderboardNames = getNames();
    if (leaderboardNames.size() > 10) leaderboardNames.erase(leaderboardNames.begin()+10, leaderboardNames.end());
    leaderboardScores = getScores();
    if (leaderboardScores.size() > 10) leaderboardScores.erase(leaderboardScores.begin()+10, leaderboardScores.end());
}

void FlashColor(GameInstanceState &gameInstance, Color color, float seconds, float relDt) {
    gameInstance.flashColor = color;
    gameInstance.flashTime = seconds;
    gameInstance.flashTimeElapsed = 0.0f;
}

void UpdateFlashColor(GameInstanceState &gameInstance, float relDt) {
    gameInstance.backgroundColor = Color{(unsigned char)Lerp(gameInstance.flashColor.r, LIGHTGRAY.r, gameInstance.flashTimeElapsed/gameInstance.flashTime), 
                            (unsigned char)Lerp(gameInstance.flashColor.g, LIGHTGRAY.g, gameInstance.flashTimeElapsed/gameInstance.flashTime), 
                            (unsigned char)Lerp(gameInstance.flashColor.b, LIGHTGRAY.b, gameInstance.flashTimeElapsed/gameInstance.flashTime), 
                            255};
    gameInstance.flashTimeElapsed += relDt / 60.0f;
}

void UpdateGameOver(GameInstanceState &gameInstance, float relDt) {
    gameInstance.floatingCone.fallSpeed += 0.01f * relDt; // Change by deltatime
    gameInstance.floatingCone.hoverDistance -= gameInstance.floatingCone.fallSpeed * relDt; // Change by deltatime
    gameInstance.camera.target = Vector3{gameInstance.floatingCone.x, gameInstance.coneYs.back().y + gameInstance.floatingCone.hoverDistance, 0.0f};
}

void UpdatePlayClassic(GameInstanceState &gameInstance, float relDt) {
    gameInstance.score = gameInstance.coneYs.size()-1;
    if ((IsKeyPressed(KEY_SPACE) && !gameSettings.enableTouchscreenControls) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && gameSettings.enableTouchscreenControls)) {
        if (gameInstance.floatingCone.x < 2.0f && gameInstance.floatingCone.x > -2.0f) {
            addNewClassicCone(gameInstance, gameSettings);
            
            gameInstance.camera.target = gameInstance.coneYs.back();
            if (gameInstance.targetFov < 90.0f) {
                gameInstance.targetFov += 1.0f * relDt; // Change by deltatime
            }
            
            gameInstance.floatingCone.x = 0.0f;
            if (gameInstance.floatingCone.speed < 1.0f) {
                gameInstance.floatingCone.speed += 0.025f;
            }
            PlaySound(coneDrop);
        } else {
        gameInstance.gameState = GAME_OVER;
        PlaySound(coneFall);
        }
    }
    if (gameInstance.floatingCone.toRight) {
        gameInstance.floatingCone.x += gameInstance.floatingCone.speed * relDt; // Change by deltatime
        if (gameInstance.floatingCone.x >= 4.0f) {
            gameInstance.floatingCone.toRight = false;
        }
    } else {
        gameInstance.floatingCone.x -= gameInstance.floatingCone.speed * relDt; // Change by deltatime
        if (gameInstance.floatingCone.x <= -4.0f) {
            gameInstance.floatingCone.toRight = true;
        }
    }
}

void UpdatePlayArcade(GameInstanceState &gameInstance, float relDt) {
    if ((IsKeyPressed(KEY_SPACE) && !gameSettings.enableTouchscreenControls) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && gameSettings.enableTouchscreenControls)) {
        if (gameInstance.floatingCone.x < 2.0f && gameInstance.floatingCone.x > -2.0f) {
            if (gameInstance.coneColor == ARCADE_CONE_RED) {
                PlaySound(redCone);
                FlashColor(gameInstance, Color{255, 0, 0, 255}, 0.5f, relDt);
                gameInstance.coneColor = randomArcadeConeColor();
                gameInstance.arcadeMessages.push_back(ArcadeMessage{"Fail!", 1.0f, false, true});
                // Remove the top 5 cones
                for (int i = 0; i < 5; i++) {
                    if (gameInstance.coneYs.size() > 1) {
                        gameInstance.coneYs.pop_back();
                        gameInstance.coneColors.pop_back();
                    }
                }
                // Decrease the score by 250
                if (gameInstance.score >= 250) {
                    gameInstance.score -= 250;
                } else {
                    gameInstance.score = 0;
                }
            } else {
                if (gameInstance.coneColor == ARCADE_CONE_GOLD) {
                    PlaySound(goldCone);
                    FlashColor(gameInstance, Color{255, 196, 0, 255}, 0.5f, relDt);
                }
                addNewArcadeCone(gameInstance);
                
                gameInstance.camera.target = gameInstance.coneYs.back();
                if (gameInstance.targetFov < 90.0f) {
                    gameInstance.targetFov += 1.0f * relDt; // Change by deltatime
                }
                
                gameInstance.floatingCone.x = 0.0f;
                if (gameInstance.floatingCone.speed < 0.5f) { // Lowered max speed for arcade mode
                    gameInstance.floatingCone.speed += 0.005f;
                }
                PlaySound(coneDrop);
            }
        } else {
            PlaySound(coneFall);
            if (gameInstance.coneColor == ARCADE_CONE_RED) {
                FlashColor(gameInstance, Color{0, 255, 0, 255}, 0.5f, relDt);
                gameInstance.coneColor = randomArcadeConeColor();
            } else {
                gameInstance.gameState = GAME_OVER;
            }
        }
    }
    if (gameInstance.floatingCone.toRight) {
        gameInstance.floatingCone.x += gameInstance.floatingCone.speed * relDt; // Change by deltatime
        if (gameInstance.floatingCone.x >= 4.0f) {
            gameInstance.floatingCone.toRight = false;
        }
    } else {
        gameInstance.floatingCone.x -= gameInstance.floatingCone.speed * relDt; // Change by deltatime
        if (gameInstance.floatingCone.x <= -4.0f) {
            gameInstance.floatingCone.toRight = true;
        }
    }
}

void UpdateCameraFOV(GameInstanceState &gameInstance) {
    if (gameInstance.targetFov > gameInstance.camera.fovy) {
        gameInstance.camera.fovy += 0.1f;
    } else if (gameInstance.targetFov < gameInstance.camera.fovy) {
        gameInstance.camera.fovy -= 0.1f;
    }
}

void DrawConeStack(GameInstanceState &gameInstance) {
    DrawCube(Vector3{1.5f, -5.0f, 1.5f}, 5.0f, 10.0f, 5.0f, GRAY);
    
    int i = 0;
    std::for_each (gameInstance.coneYs.begin(), gameInstance.coneYs.end(), [&](Vector3 coneY)
    {
        DrawCone(coneY, (gameInstance.gameState == PLAY_ARCADE || gameInstance.gameOverReturnState == PLAY_ARCADE) ? gameInstance.coneColors.at(i) : gameSettings.coneColor);
        i++;
    });
    
    // Drawing this cone for the 2nd cone stack is causing a seg fault
    DrawCone(Vector3{gameInstance.floatingCone.x, gameInstance.coneYs.back().y + gameInstance.floatingCone.hoverDistance, 0.0f}, (gameInstance.gameState == PLAY_ARCADE || gameInstance.gameOverReturnState == PLAY_ARCADE) ? gameInstance.coneColor : gameSettings.coneColor);
}

void DrawGameOver(GameInstanceState &gameInstance) {
    DrawTextCentered("Game Over", screenWidth/2, screenHeight/2-75, 50, BLACK);
    DrawTextCentered(("Score: " + std::to_string(gameInstance.score)).c_str(), screenWidth/2, screenHeight/2-25, 25, BLACK);
    //DrawTextCentered("Press Space To Play Again", screenWidth/2, screenHeight/2+75, 25, BLACK);
    if (checkScore(gameInstance.score)) {
        DrawTextCentered("Enter your name to submit your high score", screenWidth/2, screenHeight/2+8, 16, BLACK);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
        if (GuiTextBox(Rectangle{static_cast<float>(screenWidth/2-60), static_cast<float>(screenHeight/2+32), 120.0f, 24.0f}, highScoreName, 16, highScoreEditMode)) {
            highScoreEditMode = !highScoreEditMode;
        }
        GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
        if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), static_cast<float>(screenHeight/2+75), 200.0f, 50.0f}, "Play Again") == 1) {
            gameInstance.gameState = gameInstance.gameOverReturnState;
            selectLeaderboardMode(gameInstance.gameOverReturnState);
            saveScore(highScoreName, gameInstance.score);
            ResetGame(gameInstance);
        };
        if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+135, 200, 50}, "Main Menu") == 1) {
            gameInstance.gameState = MAIN_MENU;
            selectLeaderboardMode(gameInstance.gameOverReturnState);
            saveScore(highScoreName, gameInstance.score);
            ResetGame(gameInstance);
        };
    } else {
        GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
        if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+25, 200, 50}, "Play Again") == 1) {
            gameInstance.gameState = gameInstance.gameOverReturnState;
            ResetGame(gameInstance);
        };
        if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+85, 200, 50}, "Main Menu") == 1) {
            gameInstance.gameState = MAIN_MENU;
            ResetGame(gameInstance);
        };
    }
}

void DrawClassicScore(GameInstanceState &gameInstance) {
    DrawTextCentered(std::to_string(gameInstance.score).c_str(), screenWidth/2, 10, 50, BLACK);
    if (gameInstance.coneYs.size()-1 == 213) {
        // Draw image of Nate
        DrawTexture(nateTexture, screenWidth/2-nateImage.width/2, screenHeight/2-nateImage.height/2, WHITE);
    }
}

void DrawArcadeScore(GameInstanceState &gameInstance, float relDt) {
    DrawTextCentered(std::to_string(gameInstance.score).c_str(), screenWidth/2, 10, 50, BLACK);
    // Loop through arcadeMessages and reduce their time by deltatime
    for (int i = 0; i < gameInstance.arcadeMessages.size(); i++) {
        gameInstance.arcadeMessages.at(i).time -= (relDt / 60.0f);
    }
    // Draw arcadeMessages
    for (int i = 0; i < gameInstance.arcadeMessages.size(); i++) {
        Color messageColor;
        if (gameInstance.arcadeMessages.at(i).gold) {
            messageColor = Color{211, 176, 56, (unsigned char)(gameInstance.arcadeMessages.at(i).time * 255)};
        } else if (gameInstance.arcadeMessages.at(i).red) {
            messageColor = Color{255, 0, 0, (unsigned char)(gameInstance.arcadeMessages.at(i).time * 255)};
        } else {
            messageColor = Color{0, 0, 0, (unsigned char)(gameInstance.arcadeMessages.at(i).time * 255)};
        }
        DrawTextCentered(gameInstance.arcadeMessages.at(i).message.c_str(), screenWidth/2, screenHeight/2-210+(gameInstance.arcadeMessages.at(i).time * 50), 25, messageColor);
    }
    for (int i = 0; i < gameInstance.arcadeMessages.size(); i++) {
        if (gameInstance.arcadeMessages.at(i).time <= 0.0f) {
            gameInstance.arcadeMessages.erase(gameInstance.arcadeMessages.begin()+i);
        }
    }
}

void DrawMainMenu(GameInstanceState &gameInstance) {
    DrawTextCentered("Cone Stacker", screenWidth/2-35, screenHeight/2-125, 50, BLACK);
    DrawTextCentered("2", screenWidth/2+180, screenHeight/2-145, 75, BLACK);
    
    GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2-50, 200, 50}, "Play") == 1) {
        gameInstance.gameState = PLAY_MENU;
        ResetGame(gameInstance);
    };
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+10, 200, 50}, "Leaderboard") == 1) {
        gameInstance.gameState = LEADER_BOARD;
        selectLeaderboardMode(PLAY_CLASSIC);
        selectedLeaderboardMode = PLAY_CLASSIC;
        loadLeaderboardData(leaderboardNames, leaderboardScores);
    };
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+70, 200, 50}, "Options") == 1) {
        gameInstance.gameState = OPTIONS;
    };
    // Hide exit button on web
    #if !defined(PLATFORM_WEB)
        if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+130, 200, 50}, "Exit") == 1) {
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

void DrawPlayMenu(GameInstanceState &gameInstance) {
    DrawTextCentered("Cone Stacker", screenWidth/2-35, screenHeight/2-125, 50, BLACK);
    DrawTextCentered("2", screenWidth/2+180, screenHeight/2-145, 75, BLACK);
    
    GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2-50, 200, 50}, "  Classic") == 1) {
        gameInstance.gameState = PLAY_CLASSIC;
        gameInstance.gameOverReturnState = PLAY_CLASSIC;
        ResetGame(gameInstance);
    };
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+10, 200, 50}, "  Arcade") == 1) {
        gameInstance.gameState = PLAY_ARCADE;
        gameInstance.gameOverReturnState = PLAY_ARCADE;
        ResetGame(gameInstance);
    };
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+70, 200, 50}, "  Duels") == 1) {
        InitGameInstance(gameInstance);
        gameInstance.gameState = PLAY_DUELS;
        gameInstance.gameOverReturnState = PLAY_DUELS;
        ResetGame(gameInstance);
    };
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+130, 200, 50}, "Back") == 1) {
        gameInstance.gameState = MAIN_MENU;
    };
    DrawTexture(classicIconTexture, screenWidth/2-100+25, screenHeight/2-43, WHITE);
    DrawTexture(arcadeIconTexture, screenWidth/2-100+20, screenHeight/2+17, WHITE);
    DrawTexture(duelsIconTexture, screenWidth/2-100+20, screenHeight/2+77, WHITE);
}

void DrawOptions(GameInstanceState &gameInstance) {
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
                
    GuiGroupBox(Rectangle{37, 50, 640, 400}, "Options");
    
    GuiLabel(Rectangle{62, 65, 150, 25}, "Classic Cone Color");
    gameSettings.coneColorIndex = static_cast<int>(gameSettings.coneColor);
    GuiToggleGroup(Rectangle{62, 90, 150, 25}, "Traffic;Yellow;Orange;Blue", &gameSettings.coneColorIndex);
    gameSettings.coneColor = static_cast<ConeColor>(gameSettings.coneColorIndex);
    
    GuiLabel(Rectangle {207, 170, 100, 25}, "SFX Volume");
    GuiSliderBar(Rectangle{302, 175, 120, 16}, NULL, (std::to_string((int)(gameSettings.sfxVolume*100)) + "%").c_str(), &gameSettings.sfxVolume, 0, 1);
    
    GuiCheckBox(Rectangle{62, 130, 25, 25}, "Enable Touchscreen Controls", &gameSettings.enableTouchscreenControls);
    
    SetSoundVolume(coneDrop, gameSettings.sfxVolume);
    SetSoundVolume(coneFall, gameSettings.sfxVolume);
    
    if (GuiButton(Rectangle{282, 410, 150, 25}, "Back to Main Menu") == 1) {
        gameInstance.gameState = MAIN_MENU;
    }
}

void DrawLeaderboard(GameInstanceState &gameInstance) {
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
        gameInstance.gameState = MAIN_MENU;
    }
}

void UpdateGameInstance(GameInstanceState &gameInstance, RenderTexture2D &renderTexture, float relDt) {
    UpdateCamera(&gameInstance.camera, CAMERA_ORBITAL);
    
    // Update controls
    switch(gameInstance.gameState) {
        case GAME_OVER: {UpdateGameOver(gameInstance, relDt); break;}
        case PLAY_CLASSIC: {UpdatePlayClassic(gameInstance, relDt); break;}
        case PLAY_ARCADE: {UpdatePlayArcade(gameInstance, relDt); break;}
        case PLAY_DUELS: {UpdatePlayClassic(gameInstance, relDt); break;}
        default: {break;}
    }
    
    UpdateCameraFOV(gameInstance);
    if (gameInstance.flashTimeElapsed < gameInstance.flashTime) {
        UpdateFlashColor(gameInstance, relDt);
    }
    
    // Draw
    BeginTextureMode(renderTexture);
        ClearBackground(gameInstance.backgroundColor);
        
        // 3D rendering
        BeginMode3D(gameInstance.camera);
            DrawConeStack(gameInstance);
        EndMode3D();
        
        // 2D rendering
        switch(gameInstance.gameState) {
            case GAME_OVER: {DrawGameOver(gameInstance); break;}
            case PLAY_CLASSIC: {DrawClassicScore(gameInstance); break;}
            case PLAY_ARCADE: {DrawArcadeScore(gameInstance, relDt); break;}
            case PLAY_DUELS: {DrawClassicScore(gameInstance); break;}
            case MAIN_MENU: {DrawMainMenu(gameInstance); break;}
            case PLAY_MENU: {DrawPlayMenu(gameInstance); break;}
            case OPTIONS: {DrawOptions(gameInstance); break;}
            case LEADER_BOARD: {DrawLeaderboard(gameInstance); break;}
            default: {break;}
        }
    EndTextureMode();
}

void init_app() {
    InitAudioDevice();
    coneFall = LoadSound("../assets/coneFall.ogg");
    coneDrop = LoadSound("../assets/coneDrop.ogg");
    goldCone = LoadSound("../assets/gold.wav");
    redCone = LoadSound("../assets/red.wav");

    mainRenderTexture = LoadRenderTexture(screenWidth, screenHeight);
    player1RenderTexture = LoadRenderTexture(screenWidth/2, screenHeight);
    player2RenderTexture = LoadRenderTexture(screenWidth/2, screenHeight);
    
    gameSettings.coneColor = CONE_TRAFFIC;
    
    InitGameInstance(mainGameInstance);
    InitGameInstance(player2GameInstance);
    
    nateImage = LoadImage("../assets/nate.png");
    
    classicIcon = LoadImage("../assets/icons/classic.png");
    arcadeIcon = LoadImage("../assets/icons/arcade.png");
    duelsIcon = LoadImage("../assets/icons/duels.png");
    
    ImageResize(&nateImage, 576, 432);
    nateTexture = LoadTextureFromImage(nateImage);
    classicIconTexture = LoadTextureFromImage(classicIcon);
    arcadeIconTexture = LoadTextureFromImage(arcadeIcon);
    duelsIconTexture = LoadTextureFromImage(duelsIcon);
    
    selectLeaderboardMode(PLAY_CLASSIC);
}

bool app_loop() {
    float relDt = GetFrameTime() * 60.0f; // Calculate delta time in relation to 60 frames per second
    
    BeginDrawing();
        ClearBackground(BLACK);
        if (mainGameInstance.gameState != PLAY_DUELS && mainGameInstance.gameOverReturnState != PLAY_DUELS && player2GameInstance.gameState != PLAY_DUELS && player2GameInstance.gameOverReturnState != PLAY_DUELS) { 
            UpdateGameInstance(mainGameInstance, mainRenderTexture, relDt);
            DrawTextureRec(mainRenderTexture.texture, Rectangle{0, 0, (float)mainRenderTexture.texture.width, (float)-mainRenderTexture.texture.height}, Vector2{0, 0}, WHITE);
        } else {
            screenWidth = 360;
            UpdateGameInstance(mainGameInstance, player1RenderTexture, relDt);
            DrawTextureRec(player1RenderTexture.texture, Rectangle{0, 0, (float)player1RenderTexture.texture.width, (float)-player1RenderTexture.texture.height}, Vector2{0, 0}, WHITE);
            
            DrawLine(360, 0, 360, 480, BLACK);
            
            UpdateGameInstance(player2GameInstance, player2RenderTexture, relDt);
            DrawTextureRec(player2RenderTexture.texture, Rectangle{0, 0, (float)player2RenderTexture.texture.width, (float)-player2RenderTexture.texture.height}, Vector2{360, 0}, WHITE);
            screenWidth = 720;
        }
    EndDrawing();
    
    return !windowShouldClose;
}

void deinit_app() {
    UnloadSound(coneFall);
    UnloadSound(coneDrop);
    CloseAudioDevice();
}