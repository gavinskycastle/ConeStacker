#include "../libs/raylib/src/raylib.h"
#include "helper.hpp"
#include "main.hpp"

#include <filesystem>

void web_loop()
{
    static int old_w = 0, old_h = 0;

    int w = getWindowWidth();
    int h = getWindowHeight();
    if (w != old_w || h != old_h)
    {
        SetWindowSize(w, h);
    }
    app_loop();
}

int main(void)
{
    // Setup window
#if defined(PLATFORM_WEB)
    InitWindow(getBrowserWindowWidth(), getBrowserWindowHeight(), PROJECT_NAME);
#else
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(720, 480, "Cone Stacker");
    //scale window size so it always appears the same on all
    Vector2 windowScale = GetWindowScaleDPI();
    SetWindowSize((int)(720 * windowScale.x), (int)(480 * windowScale.y));
    Image windowIcon;
    if (std::filesystem::exists("../assets") == false) {
        windowIcon = LoadImage("assets/icon.png");
    } else {
        windowIcon = LoadImage("../assets/icon.png");
    }
    SetWindowIcon(windowIcon);
#endif
    // SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    init_app();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(web_loop, 0, 1);
#else
    while (app_loop() && !WindowShouldClose())
    {
    }
#endif
    deinit_app();

    // Cleanup
    CloseWindow();
    return 0;
}
