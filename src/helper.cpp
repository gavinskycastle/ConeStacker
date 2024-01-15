#include "../libs/raylib/src/raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
EM_JS(int, canvasGetWidth, (), {
  return document.getElementById("canvas").clientWidth;
});

EM_JS(int, canvasGetHeight, (), {
  return document.getElementById("canvas").clientHeight;
});
EM_JS(int, browserWindowWidth, (), {
    return window.innerWidth;
});
EM_JS(int, browserWindowHeight, (), {
    return window.innerHeight;
});
#endif

int getWindowWidth() {
#if defined(PLATFORM_WEB)
    return canvasGetWidth();
#else
    if (IsWindowFullscreen()) {
        return GetMonitorWidth(GetCurrentMonitor());
    }
    else {
        return GetScreenWidth();
    }
#endif
}

int getWindowHeight() {
#if defined(PLATFORM_WEB)
    return canvasGetHeight();
#else
    if (IsWindowFullscreen()) {
        return GetMonitorHeight(GetCurrentMonitor());
    }
    else {
        return GetScreenHeight();
    }
#endif
}

int getBrowserWindowWidth() {
#if defined(PLATFORM_WEB)
    return browserWindowWidth();
#else
    return getWindowWidth();
#endif
}

int getBrowserWindowHeight() {
#if defined(PLATFORM_WEB)
    return browserWindowHeight();
#else
    return getWindowHeight();
#endif
}
