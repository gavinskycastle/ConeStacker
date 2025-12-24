#include "../libs/raylib/src/raylib.h"
#include "helper.hpp"

#define RAYGUI_IMPLEMENTATION
#include "../libs/raygui/src/raygui.h"

#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <cmath>
#include <filesystem>

#include "main.hpp"
#include "leaderboard.hpp"
#include <raymath.h>

// Window setup
int screenWidth = 720;
const int screenHeight = 480;
bool windowShouldClose = false;

// Loading resources
std::string assetPathPrefix = "../assets/";
Sound coneFall;
Sound coneDrop;
Sound goldCone;
Sound redCone;
Sound winSound;
Image nateImage;
Image classicIcon;
Image arcadeIcon;
Image duelsIcon;
Texture2D nateTexture;
Texture2D classicIconTexture;
Texture2D arcadeIconTexture;
Texture2D duelsIconTexture;
Texture2D leaderboardGreyedOutTexture;

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

// Duels setup
float duelsTimer = 0.0f;
int duelsRoundCount = 0;
bool duelsPointAwarded = false;
bool duelsVictorAnnounced = false;

void DrawCone(Vector3 position, int color) {
    static Model trafficConeModel = LoadModel((assetPathPrefix + "trafficCone/tinker.obj").c_str());
    static Model yellowConeModel = LoadModel((assetPathPrefix + "yellowCone/tinker.obj").c_str());
    static Model orangeConeModel = LoadModel((assetPathPrefix + "orangeCone/tinker.obj").c_str());
    static Model blueConeModel = LoadModel((assetPathPrefix + "blueCone/tinker.obj").c_str());
    
    static Model arcadeGoldConeModel = LoadModel((assetPathPrefix + "goldCone/tinker.obj").c_str());
    static Model arcadeRedConeModel = LoadModel((assetPathPrefix + "redCone/tinker.obj").c_str());
    
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
    gameInstance.camera.position = Vector3{ 10.0f, 10.0f, 10.0f }; // Camera position
    gameInstance.camera.target = Vector3{ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    gameInstance.camera.up = Vector3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    gameInstance.camera.fovy = 45.0f;                              // Camera field-of-view Y
    gameInstance.camera.projection = CAMERA_PERSPECTIVE;           // Camera projection type
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

bool detectClicks(GameInstanceState &gameInstance) {
    if (gameInstance.gameState == PLAY_DUELS) {
        // TODO: Add multitouch support
        if (gameInstance.player == 1) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (GetMousePosition().x < 360) {
                    return true;
                }
            }
        } else {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (GetMousePosition().x > 360) {
                    return true;
                }
            }
        }
        return false;
    } else {
        return IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    }
}

void FlashColor(GameInstanceState &gameInstance, Color color, float seconds, float relDt) {
    gameInstance.flashColor = color;
    gameInstance.flashTime = seconds;
    gameInstance.flashTimeElapsed = 0.0f;
}

bool PlayerPressed(GameInstanceState &gameInstance) {
    if (gameInstance.gameState == PLAY_DUELS) {
        return (gameInstance.player == 1 ? IsKeyPressed(KEY_LEFT_SHIFT) : IsKeyPressed(KEY_RIGHT_SHIFT)) && !gameSettings.enableTouchscreenControls;
    } else {
        return IsKeyPressed(KEY_SPACE) && !gameSettings.enableTouchscreenControls;
    }
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
    if (PlayerPressed(gameInstance) || (detectClicks(gameInstance) && gameSettings.enableTouchscreenControls)) {
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
    if (PlayerPressed(gameInstance) || (detectClicks(gameInstance) && gameSettings.enableTouchscreenControls)) {
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
        DrawCone(coneY, (gameInstance.gameOverReturnState == PLAY_ARCADE || gameInstance.gameOverReturnState == PLAY_DUELS) ? gameInstance.coneColors.at(i) : gameSettings.coneColor);
        i++;
    });
    
    // Drawing this cone for the 2nd cone stack is causing a seg fault
    DrawCone(Vector3{gameInstance.floatingCone.x, gameInstance.coneYs.back().y + gameInstance.floatingCone.hoverDistance, 0.0f}, (gameInstance.gameOverReturnState == PLAY_ARCADE || gameInstance.gameOverReturnState == PLAY_DUELS) ? gameInstance.coneColor : gameSettings.coneColor);
}

void DrawGameOver(GameInstanceState &gameInstance) {
    DrawTextCentered("Game Over", screenWidth/2, screenHeight/2-75+(gameInstance.gameOverReturnState == PLAY_DUELS ? 25 : 0), 50, BLACK);
    DrawTextCentered(("Score: " + std::to_string(gameInstance.score)).c_str(), screenWidth/2, screenHeight/2-25+(gameInstance.gameOverReturnState == PLAY_DUELS ? 25 : 0), 25, BLACK);
    //DrawTextCentered("Press Space To Play Again", screenWidth/2, screenHeight/2+75, 25, BLACK);
    if (checkScore(gameInstance.score)) {
        if (gameInstance.gameOverReturnState != PLAY_DUELS) {
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
        }
    } else if (gameInstance.gameOverReturnState != PLAY_DUELS) {
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

void DrawArcadeMessages(std::vector<ArcadeMessage> &arcadeMessages, float relDt, int x, int y) {
    // Loop through arcadeMessages and reduce their time by deltatime
    for (int i = 0; i < arcadeMessages.size(); i++) {
        arcadeMessages.at(i).time -= (relDt / 60.0f);
    }
    // Draw arcadeMessages
    for (int i = 0; i < arcadeMessages.size(); i++) {
        Color messageColor;
        if (arcadeMessages.at(i).gold) {
            messageColor = Color{211, 176, 56, (unsigned char)(arcadeMessages.at(i).time * 255)};
        } else if (arcadeMessages.at(i).red) {
            messageColor = Color{255, 0, 0, (unsigned char)(arcadeMessages.at(i).time * 255)};
        } else {
            messageColor = Color{0, 0, 0, (unsigned char)(arcadeMessages.at(i).time * 255)};
        }
        DrawTextCentered(arcadeMessages.at(i).message.c_str(), x, y+(arcadeMessages.at(i).time * 50), 25, messageColor);
    }
    for (int i = 0; i < arcadeMessages.size(); i++) {
        if (arcadeMessages.at(i).time <= 0.0f) {
            arcadeMessages.erase(arcadeMessages.begin()+i);
        }
    }
}

void DrawArcadeScore(GameInstanceState &gameInstance, float relDt) {
    DrawTextCentered(std::to_string(gameInstance.score).c_str(), screenWidth/2, 10, 50, BLACK);
    DrawArcadeMessages(gameInstance.arcadeMessages, relDt, screenWidth/2, screenHeight/2-210);
}

void DrawDuelsScore(float relDt) {
    DrawTextCentered(std::to_string(mainGameInstance.duelsScore).c_str(), screenWidth/2-30, 10, 50, BLACK);
    DrawTextCentered(std::to_string(player2GameInstance.duelsScore).c_str(), screenWidth/2+30, 10, 50, BLACK);
    DrawRectangleLines(screenWidth/2-60, 0, 120, 60, BLACK);
    DrawLine(360, 0, 360, 480, BLACK);
    
    DrawArcadeMessages(mainGameInstance.duelsMessages, relDt, screenWidth/2-30, screenHeight/2-210);
    DrawArcadeMessages(player2GameInstance.duelsMessages, relDt, screenWidth/2+30, screenHeight/2-210);
    
    if (duelsTimer >= 1.0f) {
        if (mainGameInstance.duelsScore >= 5 && player2GameInstance.duelsScore < 5) {
            DrawTextCentered("WINNER", screenWidth/4, 10, 50, DARKGREEN);
            DrawTextCentered("LOSER", (screenWidth/2) + (screenWidth/4), 10, 50, MAROON);
        } else if (player2GameInstance.duelsScore >= 5 && mainGameInstance.duelsScore < 5) {
            DrawTextCentered("LOSER", screenWidth/4, 10, 50, MAROON);
            DrawTextCentered("WINNER", (screenWidth/2) + (screenWidth/4), 10, 50, DARKGREEN);
        }
    }
}

void DrawMainMenu(GameInstanceState &gameInstance) {
    DrawTextCentered("Cone Stacker", screenWidth/2, screenHeight/2-125, 50, BLACK);
    
    GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2-50, 200, 50}, "Play") == 1) {
        gameInstance.gameState = PLAY_MENU;
    };
    // Grey out leaderboard button on web
    #if !defined(PLATFORM_WEB)
        if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+10, 200, 50}, "Leaderboard") == 1) {
            gameInstance.gameState = LEADER_BOARD;
            selectLeaderboardMode(PLAY_CLASSIC);
            selectedLeaderboardMode = PLAY_CLASSIC;
            loadLeaderboardData(leaderboardNames, leaderboardScores);
        };
    #else
        DrawTexture(leaderboardGreyedOutTexture, screenWidth/2-100, screenHeight/2+10, WHITE);
    #endif
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+70, 200, 50}, "Options") == 1) {
        gameInstance.gameState = OPTIONS;
    };
    // Hide exit button on web
    #if !defined(PLATFORM_WEB)
        if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+130, 200, 50}, "Exit") == 1) {
            windowShouldClose = true;
        };
    #endif
    
    // if (GuiButton(Rectangle {32-2, screenHeight-100-1, 52, 52}, "") == 1) {
    //     // Profile button hit
    // }
    // GuiDrawIcon(ICON_PLAYER, 32, screenHeight-100, 3, GRAY);
    // if (GuiButton(Rectangle {102-2, screenHeight-100-1, 52, 52}, "") == 1) {
    //     // Messages button hit
    // }
    // GuiDrawIcon(ICON_MAILBOX, 102, screenHeight-100, 3, GRAY);
    
    DrawText("Made by Gavin P - v0.2.0", 5, screenHeight-20, 15, BLACK);
}

void DrawPlayMenu(GameInstanceState &gameInstance) {
    DrawTextCentered("Cone Stacker", screenWidth/2, screenHeight/2-125, 50, BLACK);
    
    GuiSetStyle(DEFAULT, TEXT_SIZE, 25);
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2-50, 200, 50}, "  Classic") == 1) {
        gameInstance.gameState = PLAY_CLASSIC;
        gameInstance.gameOverReturnState = PLAY_CLASSIC;
    };
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+10, 200, 50}, "  Arcade") == 1) {
        gameInstance.gameState = PLAY_ARCADE;
        gameInstance.gameOverReturnState = PLAY_ARCADE;
    };
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+70, 200, 50}, "  Duels") == 1) {
        ResetGame(gameInstance);
        ResetGame(player2GameInstance);
        gameInstance.gameState = PLAY_DUELS;
        gameInstance.gameOverReturnState = PLAY_DUELS;
        player2GameInstance.gameState = PLAY_DUELS;
        player2GameInstance.gameOverReturnState = PLAY_DUELS;
        gameInstance.duelsScore = 0;
        player2GameInstance.duelsScore = 0;
    };
    if (GuiButton(Rectangle {static_cast<float>(screenWidth/2-100), screenHeight/2+130, 200, 50}, "Back") == 1) {
        gameInstance.gameState = MAIN_MENU;
    };
    DrawTexture(classicIconTexture, screenWidth/2-100+25, screenHeight/2-43, WHITE);
    DrawTexture(arcadeIconTexture, screenWidth/2-100+20, screenHeight/2+17, WHITE);
    DrawTexture(duelsIconTexture, screenWidth/2-100+20, screenHeight/2+77, WHITE);
    
    DrawText("Made by Gavin P - v0.2.0", 5, screenHeight-20, 15, BLACK);
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
        case PLAY_DUELS: {UpdatePlayArcade(gameInstance, relDt); break;}
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
            case PLAY_DUELS: {DrawArcadeScore(gameInstance, relDt); break;}
            case MAIN_MENU: {DrawMainMenu(gameInstance); break;}
            case PLAY_MENU: {DrawPlayMenu(gameInstance); break;}
            case OPTIONS: {DrawOptions(gameInstance); break;}
            case LEADER_BOARD: {DrawLeaderboard(gameInstance); break;}
            default: {break;}
        }
    EndTextureMode();
}

void UpdatePlayDuels(float relDt) {
    screenWidth = 360;
    UpdateGameInstance(mainGameInstance, player1RenderTexture, relDt);
    DrawTextureRec(player1RenderTexture.texture, Rectangle{0, 0, (float)player1RenderTexture.texture.width, (float)-player1RenderTexture.texture.height}, Vector2{0, 0}, WHITE);
    UpdateGameInstance(player2GameInstance, player2RenderTexture, relDt);
    DrawTextureRec(player2RenderTexture.texture, Rectangle{0, 0, (float)player2RenderTexture.texture.width, (float)-player2RenderTexture.texture.height}, Vector2{360, 0}, WHITE);
    screenWidth = 720;
    
    if (mainGameInstance.gameState == GAME_OVER && player2GameInstance.gameState == GAME_OVER) {
        duelsTimer += relDt / 60.0f;
        if (duelsTimer >= 5.0f) {
            if (duelsVictorAnnounced) {
                mainGameInstance.gameState = MAIN_MENU;
                player2GameInstance.gameState = MAIN_MENU;
                mainGameInstance.gameOverReturnState = MAIN_MENU;
                player2GameInstance.gameOverReturnState = MAIN_MENU;
                ResetGame(mainGameInstance);
                ResetGame(player2GameInstance);
                duelsVictorAnnounced = false;
            } else {
                duelsRoundCount++;
                duelsTimer = 0.0f;
                ResetGame(mainGameInstance);
                ResetGame(player2GameInstance);
                mainGameInstance.gameState = PLAY_DUELS;
                player2GameInstance.gameState = PLAY_DUELS;
                duelsPointAwarded = false;
            }
        } else if (duelsTimer >= 1.0f) {
            if (!duelsVictorAnnounced) {
                if (mainGameInstance.duelsScore >= 5 && player2GameInstance.duelsScore < 5) {
                    PlaySound(winSound);
                    duelsVictorAnnounced = true;
                } else if (player2GameInstance.duelsScore >= 5 && mainGameInstance.duelsScore < 5) {
                    PlaySound(winSound);
                    duelsVictorAnnounced = true;
                } else {
                    DrawText(("Next round in "+std::to_string(4-(int)duelsTimer)).c_str(), 10, screenHeight-35, 25, BLACK);
                }
            } else {
                DrawText(("Exiting in "+std::to_string(4-(int)duelsTimer)).c_str(), 10, screenHeight-35, 25, BLACK);
            }
        } else if (duelsTimer >= 0.5f) {
            if (!duelsPointAwarded) {
                if (mainGameInstance.score > player2GameInstance.score) {
                    mainGameInstance.duelsScore++;
                    mainGameInstance.duelsMessages.push_back(ArcadeMessage{"+1", 1.0f});
                    PlaySound(goldCone);
                    FlashColor(mainGameInstance, Color{0, 255, 0, 255}, 1.0f, relDt);
                } else if (player2GameInstance.score > mainGameInstance.score) {
                    player2GameInstance.duelsScore++;
                    player2GameInstance.duelsMessages.push_back(ArcadeMessage{"+1", 1.0f});
                    PlaySound(goldCone);
                    FlashColor(player2GameInstance, Color{0, 255, 0, 255}, 1.0f, relDt);
                } else {
                    mainGameInstance.duelsMessages.push_back(ArcadeMessage{"Tie", 1.0f});
                    player2GameInstance.duelsMessages.push_back(ArcadeMessage{"Tie", 1.0f});
                    PlaySound(redCone);
                    FlashColor(mainGameInstance, Color{255, 0, 0, 255}, 1.0f, relDt);
                    FlashColor(player2GameInstance, Color{255, 0, 0, 255}, 1.0f, relDt);
                }
                duelsPointAwarded = true;
            }
        }
    }
}

void init_app() {
    if (std::filesystem::exists("../assets") == false) {
        assetPathPrefix = "assets/";
    }
    
    InitAudioDevice();
    coneFall = LoadSound((assetPathPrefix + "coneFall.ogg").c_str());
    coneDrop = LoadSound((assetPathPrefix + "coneDrop.ogg").c_str());
    goldCone = LoadSound((assetPathPrefix + "gold.wav").c_str());
    redCone = LoadSound((assetPathPrefix + "red.wav").c_str());
    winSound = LoadSound((assetPathPrefix + "win.ogg").c_str());

    mainRenderTexture = LoadRenderTexture(screenWidth, screenHeight);
    player1RenderTexture = LoadRenderTexture(screenWidth/2, screenHeight);
    player2RenderTexture = LoadRenderTexture(screenWidth/2, screenHeight);
    
    gameSettings.coneColor = CONE_TRAFFIC;
    
    ResetGame(mainGameInstance);
    ResetGame(player2GameInstance);
    mainGameInstance.player = 1;
    player2GameInstance.player = 2;
    
    nateImage = LoadImage((assetPathPrefix + "nate.png").c_str());
    
    classicIcon = LoadImage((assetPathPrefix + "icons/classic.png").c_str());
    arcadeIcon = LoadImage((assetPathPrefix + "icons/arcade.png").c_str());
    duelsIcon = LoadImage((assetPathPrefix + "icons/duels.png").c_str());
    
    leaderboardGreyedOutTexture = LoadTexture((assetPathPrefix + "leaderboardGreyedOut.png").c_str());
    
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
            UpdatePlayDuels(relDt);
            DrawDuelsScore(relDt);
        }
    EndDrawing();
    
    return !windowShouldClose;
}

void deinit_app() {
    UnloadSound(coneFall);
    UnloadSound(coneDrop);
    CloseAudioDevice();
}