#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getopt

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "app.h"
#include "ui.h"


SDL_Window *ui_init_window(App *app) {
    SDL_Window *win = NULL;

    int res = SDL_Init(SDL_INIT_VIDEO);
    EXIT_IF_F(res != 0, "SDL failed to initialise: %s\n", SDL_GetError());

    /* Creates a SDL window */
    win = SDL_CreateWindow(
        app->name,
        (int) app->window.x,
        (int) app->window.y,
        (int) app->window.w,
        (int) app->window.h,
    0);
    EXIT_IF_F(win == NULL, "SDL window failed to initialise: %s\n", SDL_GetError());

    return win;
}

SDL_Renderer *ui_init_renderer(App *app, SDL_Window *window) {
    SDL_Renderer *renderer = NULL;

    renderer = SDL_CreateRenderer(window,  -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    EXIT_IF_F(renderer == NULL, "SDL failed to initialise: %s\n", SDL_GetError());

    SDL_SetRenderDrawColor(renderer, app->bg_color.r, app->bg_color.g, app->bg_color.b, app->bg_color.a);
    SDL_RenderClear(renderer);

    return renderer;
}

TTF_Font *ui_init_font(App *app) {
    TTF_Font *font = NULL;

    int res = TTF_Init();
    EXIT_IF_F( res < 0, "SDL_ttf failed to initialise: %s\n", SDL_GetError());

    font = TTF_OpenFont(app->font_path, app->font_sz);
    EXIT_IF_F(font == NULL, "SDL_ttf failed to load font:: %s\n", SDL_GetError());

    return font;
}

/**
 * render a text message
 */
SDL_Texture* ui_render_text(const char *message, SDL_Renderer *renderer, TTF_Font *font, SDL_Color color) {
    if (!renderer || !font) {
        return NULL;
    }

    SDL_Surface *surface = TTF_RenderText_Solid(font, message, color);
    if (!surface) {
        return NULL; // TODO LOG_ERROR
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        return NULL; // TODO LOG_ERROR
    }

    // clean up the surface
    SDL_FreeSurface(surface);

    return texture;
}

void ui_exit(App *app, SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font) {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_Quit();
}

////
// widgets
////

int ui_draw_status_bar(App *app, SDL_Renderer *renderer, TTF_Font *font) {
    if (!app || !renderer || !font ) {
        return -1;
    }

    char msg[128] = {0};
    snprintf(msg, 128, "%s", (app->paused) ? "paused" : ">>>");

    SDL_Texture *texture = ui_render_text(msg, renderer, font, app->fg_color);
    if (!texture){
        return 1; // TODO log error
    }

    SDL_Rect trect = {0};
    SDL_QueryTexture(texture, NULL, NULL, &trect.w, &trect.h);

    trect.x = 5;
    trect.y = app->window.h - trect.h -5;

    return SDL_RenderCopy(renderer, texture, NULL, &trect);
}
