#ifndef __WORLD_H__
#define __WORLD_H__

#include "app.h"
#include "vec2.h"

#define WORLD_POP_MAX 1000
#define GRAVITY 0.01 // 0.000000000066742f // Gravitational constant

// forward declarations

typedef struct Creature Creature;
typedef struct QuadTree QuadTree;
typedef struct RuleSet RuleSet;

typedef struct World {
    Vec2 nw; // north-west corner of the world (min)
    Vec2 se; // south-east corner of the world (max)
    size_t len;
    Creature *population[WORLD_POP_MAX];
    QuadTree *qtree;
    RuleSet *rules;
} World;

#define WORLD_WIDTH(w) fabs(w->se.x - w->nw.x)
#define WORLD_HEIGHT(w) fabs(w->se.y - w->nw.y)

// Live cycle

World *world_create(size_t len, Vec2 nw, Vec2 se);
void world_destroy(World *world);

// Debug

void world_print(FILE *fp, World *world);

// Main loop

int world_update(App *app, World *world);
int world_draw(App *app, World *world);

// Rules

typedef struct Rule {
    int left;
    int right;
    float val;
} Rule;

typedef struct RuleSet {
    size_t len;
    Rule **rules;
} RuleSet;

RuleSet *rules_create();
Rule *rules_set(RuleSet *rules, int left, int right, float val);
Rule *rules_get(RuleSet *rules, int left, int right);
void rules_destroy(RuleSet *rules);
#endif
