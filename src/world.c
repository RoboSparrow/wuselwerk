#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "app.h"
#include "ui.h"


int world_update(App *app) {
    int res = 0;
    // TODO
    return res;
}

int world_draw(App *app, SDL_Renderer *renderer, TTF_Font *font) {
    if (!app || !renderer || !font) {
        return -1;
    }
    int res = 0;

    res = ui_draw_status_bar(app, renderer, font);
    return res;
}
