#ifndef __UI_H__
#define __UI_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "app.h"
#include "world.h"
#include "utils.h"

SDL_Window *ui_init_window(App *app, World *world);
SDL_Renderer *ui_init_renderer(App *app, SDL_Window *window);
TTF_Font *ui_init_font(App *app);

void ui_exit(App *app, SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font);

SDL_Texture* ui_render_text(const char *message, SDL_Renderer *renderer, TTF_Font *font, SDL_Color color);
int ui_draw_status_bar(App *app, World *world, SDL_Renderer *renderer, TTF_Font *font);

#endif
