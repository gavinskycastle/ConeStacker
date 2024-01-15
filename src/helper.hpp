#ifndef HELPER_HPP
#define HELPER_HPP

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "../libs/raylib/src/raylib.h"

#ifndef PROJECT_NAME
#define PROJECT_NAME "UwU"
#endif

#ifndef VERSION
#define VERSION "N/A"
#endif

int getWindowWidth();
int getWindowHeight();
int getBrowserWindowWidth();
int getBrowserWindowHeight();

#endif
