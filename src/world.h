#ifndef __WORLD_H__
#define __WORLD_H__

#include <SDL2/SDL_ttf.h>

#include "vec2.h"
#include "app.h"
#include "world.h"

#define WORLD_POP_MAX 100


typedef struct App App; // forward declaration (TODO, why neccessary here?)
typedef struct Creature Creature; // forward declaration (TODO, why neccessary here?)

typedef struct World {
    Vec2 nw;   // north-west corner of the world (min)
    Vec2 se;   // south-east corner of the world (max)
    size_t len;
    Creature *population[WORLD_POP_MAX];
} World;

#define WORLD_WIDTH(w)  fabs(w->se.x - w->nw.x)
#define WORLD_HEIGHT(w) fabs(w->se.y - w->nw.y)

void world_print(FILE *fp, World *world);

int world_update(App *app);
int world_draw(App *app, SDL_Renderer *renderer, TTF_Font *font);

void world_destroy(World *world);
#endif
