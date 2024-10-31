#ifndef __WORLD_H__
#define __WORLD_H__

#include <SDL2/SDL_ttf.h>
#include "app.h"


#define POP_MAX 100


typedef struct App App; // forward declaration (TODO, why neccessary here?)
typedef struct Creature Creature; // forward declaration (TODO, why neccessary here?)

typedef struct Population {
    size_t len;
    Creature *members[POP_MAX];
} Population;

void pop_destroy(Population *pop);
void pop_print(FILE *fp, Population *pop);


int world_update(App *app);
int world_draw(App *app, SDL_Renderer *renderer, TTF_Font *font);

#endif
