#ifndef __APP_H__
#define __APP_H__

#include <SDL2/SDL.h>

#define APP_MAX_FPS 60
#define APP_BUILD_INFO_PATH "./build"

typedef struct App {
    char name[128];
    char version[128];
    SDL_Rect window;
    SDL_Color bg_color;
    SDL_Color fg_color;

    char *font_path;
    unsigned int font_sz; // font size (pt)

    unsigned int fps; // seconds
    char running;
    char paused;
} App;

int app_get_version(App *app);
void app_destroy(App *app);

#endif
