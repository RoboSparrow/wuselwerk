#ifndef __CRT_H__
#define __CRT_H__

#include "app.h"
#include "world.h"
#include "qtree.h"
#include "vec2.h"

#define CRT_MIN_SIZE 10.f
#define CRT_MIN_AGILITY 0
#define CRT_MIN_PERCEPTION 0

#define CRT_MIN_DIST 5.f
#define CRT_POS_NONE -1000.f

#define CRT_NAME_LEN 16

typedef struct QuadList QuadList; // forward decl, TODO remove, after circlular includes is resolved

////
// Crt
////

typedef enum CrtType {
    CRT_TYPE_NONE,
    CRT_TYPE_HERBIVORE,
    CRT_TYPE_CARNIVORE,
    CRT_TYPE_MAX
} CrtType;
extern const char crt_type_names[][32];
#define CRT_TYPE_NAME(t) ((t >= 0 && t < CRT_TYPE_MAX ) ? crt_type_names[t] : "<UNDEFINED>" )

typedef enum CrtStatus {
    CRT_STATUS_NONE,
    CRT_STATUS_DEAD,
    CRT_STATUS_ALIVE,
    CRT_STATUS_MAX
} CrtStatus;
extern const char crt_status_names[][32];
#define CRT_STATUS_NAME(s) ((s >= 0 && s < CRT_STATUS_MAX ) ? crt_status_names[s] : "<UNDEFINED>" )

typedef struct Creature {
    unsigned int id;
    char name[CRT_NAME_LEN];

    CrtType type;
    CrtStatus status;

    float agility;
    float size;

    float perception;

    Vec2 pos;
    Vec2 targ;
} Creature;

#define CRT_INIT(id) {id, {0}, CRT_TYPE_NONE, CRT_STATUS_NONE, 0, 0, 0, {CRT_POS_NONE, CRT_POS_NONE}, {CRT_POS_NONE, CRT_POS_NONE}}

// Live Cycle

Creature *crt_create(unsigned int id);
Creature *crt_birth(int id, char *name, CrtType type, Vec2 pos);
void crt_destroy(Creature *crt);

int crt_random_targ(Creature *crt, World *world, float max_radius);

// Debug

void crt_print(FILE *fp, Creature *crt);

// Main loop

int crt_update(Creature *crt, App *app, World *world);
int crt_draw(Creature *crt, App *app, World *world);

////
// Relationships
////

QuadList *crt_find_neighbours(Creature *crt, App *app, World *world, QuadList *list) ;
int crt_draw_neighbours(Creature *crt, QuadList *list, App *app, World *world);

#endif
