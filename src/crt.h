#ifndef __CRT_H__
#define __CRT_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "app.h"
#include "world.h"
#include "vec2.h"

#define CRT_MIN_W 10.f
#define CRT_MIN_H 10.f
#define CRT_MIN_AGILITY 0
#define CRT_MIN_PERCEPTION 0

#define CRT_MIN_DIST 5.f
#define CRT_POS_NONE -1000.f

#define CRT_NAME_LEN 16

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
    float w;
    float h;

    float perception;

    Vec2 pos;
    Vec2 targ;

} Creature;

#define CRT_INIT(id) {id, {0}, CRT_TYPE_NONE, CRT_STATUS_NONE, 0, 0, 0, 0, {CRT_POS_NONE, CRT_POS_NONE}, {CRT_POS_NONE, CRT_POS_NONE}}

// Live Cycle

Creature *crt_create(unsigned int id);
Creature *crt_birth(int id, char *name, CrtType type, Vec2 pos);
void crt_destroy(Creature *crt);

int crt_random_targ(Creature *crt, App *app, float max_radius);

// Debug

void crt_print(FILE *fp, Creature *crt);

// Main loop

int crt_update(Creature *crt, App *app, World *world);
int crt_draw(Creature *crt, App *app, World *world, SDL_Renderer *renderer, TTF_Font *font);

////
// CrtList
////

typedef struct CrtList {
  size_t len;
  size_t grow;
  size_t max;
  Creature **members;
} CrtList;

CrtList *crt_list_create(size_t max);
CrtList *crt_list_append(CrtList *list, Creature *crt);
void crt_list_reset(CrtList *list);
void crt_list_print(FILE *fp, CrtList *list);
void crt_list_destroy(CrtList *list);

////
// Relationships
////

CrtList *crt_find_neighbours(Creature *crt, App *app, World *world, CrtList *list);
int crt_draw_neighbours(Creature *crt, CrtList *list, App *app, World *world, SDL_Renderer *renderer, TTF_Font *font);

#endif
