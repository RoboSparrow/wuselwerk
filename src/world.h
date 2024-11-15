#ifndef __WORLD_H__
#define __WORLD_H__

#include "app.h"
#include "vec2.h"

#define WORLD_POP_MAX 1000

// forward declarations

typedef struct Creature Creature;
typedef struct QuadTree QuadTree;

typedef struct World {
    Vec2 nw;   // north-west corner of the world (min)
    Vec2 se;   // south-east corner of the world (max)
    size_t len;
    Creature *population[WORLD_POP_MAX];
    QuadTree *qtree;
} World;

#define WORLD_WIDTH(w)  fabs(w->se.x - w->nw.x)
#define WORLD_HEIGHT(w) fabs(w->se.y - w->nw.y)

// Live cycle

World *world_create(size_t len, Vec2 nw, Vec2 se);
void world_destroy(World *world);

// Debug

void world_print(FILE *fp, World *world);

// Main loop

int world_update(App *app, World *world);
int world_draw(App *app, World *world);

#endif
