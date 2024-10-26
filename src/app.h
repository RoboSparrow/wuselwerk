#ifndef __APP_H__
#define __APP_H__

#include <SDL2/SDL.h>
#include "pop.h"

typedef struct Population Population; // forward declaration (TODO, why neccessary here?)

#define APP_MAX_FPS 60

typedef struct App {
    char name[128];
    SDL_Rect window;
    SDL_Color bg_color;
    SDL_Color fg_color;

    char *font_path;
    unsigned int font_sz; // font size (pt)

    unsigned int fps; // seconds
    char running;
    char paused;
} App;

#endif
