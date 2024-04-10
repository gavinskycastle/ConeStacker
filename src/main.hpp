#ifndef APP_HPP
#define APP_HPP

// Load asset and initialized stuffs here
void init_app();

// The main loop (return false to end loop)
bool app_loop();

// Free up allocated memory
void deinit_app();

#endif