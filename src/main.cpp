#include "main.hpp"
#include "../libs/raylib/src/raylib.h"
#include "helper.hpp"

Texture2D texture;

void init_app()
{
    texture = LoadTexture("assets/test.png");
}

bool app_loop()
{

    BeginDrawing();

    ClearBackground(RAYWHITE);

    const int texture_x = getWindowWidth() / 2 - texture.width / 2;
    const int texture_y = getWindowHeight() / 2 - texture.height / 2;
    DrawTexture(texture, texture_x, texture_y, WHITE);

    const char *text = "OMG! IT WORKS!";
    const Vector2 text_size = MeasureTextEx(GetFontDefault(), text, 20, 1);
    DrawText(text, getWindowWidth() / 2.0 - text_size.x / 2, texture_y + texture.height + text_size.y + 10, 20, BLACK);

    EndDrawing();

    return true;
}

void deinit_app()
{
    UnloadTexture(texture);
}