#ifndef __WORLD_H__
#define __WORLD_H__

#include "app.h"

int world_update(App *app);
int world_draw(App *app, SDL_Renderer *renderer, TTF_Font *font);

#endif
