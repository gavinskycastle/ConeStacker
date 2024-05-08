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

// Load asset and initialized stuffs here
void init_app();

// The main loop (return false to end loop)
bool app_loop();

// Free up allocated memory
void deinit_app();

#endif