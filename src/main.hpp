#include <string>
#include <vector>
#include "../libs/raylib/src/raylib.h"

#ifndef APP_HPP
#define APP_HPP

enum GameState {
    PLAY_CLASSIC,
    PLAY_ARCADE,
    PLAY_RHYTHM,
    PLAY_DUELS,
    GAME_OVER,
    MAIN_MENU,
    PLAY_MENU,
    OPTIONS,
    LEADER_BOARD,
};

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
    bool gold = false;
    bool red = false;
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

struct GameInstanceState {
    int player;
    // Background color setup
    Color backgroundColor = LIGHTGRAY;
    Color flashColor = LIGHTGRAY;
    float flashTime = 0.0f;
    float flashTimeElapsed = 0.0f;
    // Camera setup
    Camera3D camera = { 0 };
    float targetFov = 45.0f;
    // Game instance state setup
    GameState gameState = MAIN_MENU;
    GameState gameOverReturnState = PLAY_CLASSIC;
    FloatingCone floatingCone;
    ConeColor coneColor = CONE_TRAFFIC;
    std::vector<Vector3> coneYs;
    std::vector<ConeColor> coneColors = {CONE_TRAFFIC};
    std::vector<ArcadeMessage> arcadeMessages;
    int score = 0;
};

// Load asset and initialized stuffs here
void init_app();

// The main loop (return false to end loop)
bool app_loop();

// Free up allocated memory
void deinit_app();

#endif