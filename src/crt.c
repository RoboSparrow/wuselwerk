#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "world.h"
#include "app.h"
#include "crt.h"
#include "qtree.h" // toto remove
#include "vec2.h"
#include "utils.h"

#define CRT_EVT_NONE = 0;
#define CRT_EVT_TARG_REACHED = 1;
#define CRT_EVT_BOUNDS_REACHED = 2;

const char crt_type_names[][32] = {"CRT_TYPE_NONE", "CRT_TYPE_HERBIVORE", "CRT_TYPE_CARNIVORE"};
const char crt_status_names[][32] = {"CRT_STATUS_NONE", "CRT_STATUS_DEAD", "CRT_STATUS_ALIVE"};

////
// Crt
////

Creature *crt_create(unsigned int id) {
    // Creature crt = { id, {0}, CRT_TYPE_NONE, CRT_STATUS_NONE, 0, 0, 0, {CRT_POS_NONE, CRT_POS_NONE}, {CRT_POS_NONE, CRT_POS_NONE}  };
    Creature *crt = calloc(sizeof(Creature), 1);
    EXIT_IF_F(crt == NULL, "failed to allocate memory for creature %d", id);

    crt->id = id;
    crt->pos = (Vec2){CRT_POS_NONE, CRT_POS_NONE};
    crt->targ = (Vec2){CRT_POS_NONE, CRT_POS_NONE};
    return crt;
}

Creature *crt_birth(int id, char *name, CrtType type, Vec2 pos) {
    Creature *crt = crt_create(id);
    strncpy(crt->name, name, CRT_NAME_LEN);

    crt->type = type;
    crt->status = CRT_STATUS_ALIVE;

    crt->agility = CRT_MIN_AGILITY;
    crt->w = CRT_MIN_W;
    crt->h = CRT_MIN_H; // todo, per type

    crt->perception = CRT_MIN_PERCEPTION;

    crt->pos = pos; // valid cp struct!
    crt->targ = pos;
    return crt;
}

void crt_destroy(Creature *crt) {
    // provision for future complexities
    freez(crt);
}

int crt_random_targ(Creature *crt, App *app, float max_radius) {
    if (!crt || !app) {
        return 1;
    }

    Vec2 delta = vec2_rand_from(crt->pos, max_radius); // TODO radius perception?

    delta = vec2_add(crt->pos, delta);
    delta.x = clamp_f(delta.x, 0, app->window.w);
    delta.y = clamp_f(delta.y, 0, app->window.h);

    crt->targ = delta;
    return 0;
}

/**
 * Main loop: update
 */
int crt_update(Creature *crt, App *app, World *world) {
    if (!crt || !app || !world) {
        return 1;
    }

    // compute speed and progess linear
    float speed = crt->agility * 25.0; // TODO dynamic

    Vec2 delta = vec2_sub(crt->pos, crt->targ);
    float mag = vec2_mag(delta);
    Vec2 norm = vec2_norm(delta);

    crt->pos.x -= speed * norm.x;
    crt->pos.y -= speed * norm.y;

    // overshoot
    if (fabs(mag) < speed) {
        crt_random_targ(crt, app, 200.f); //TODO radus, perception
    }

    return 0;
}

/**
 * Main loop: draw
 */
void crt_print(FILE *fp, Creature *crt) {
    if(!fp) {
        return;
    }
    if (!crt) {
        fprintf(fp, "<NULL>\n");
        return;
    }

    fprintf(fp,
        "{"
        " id: %d,"
        " name: \"%s\","
        " type: %s,"
        " status: %s,"
        " agility: %f,"
        " width: %f,"
        " height: %f,"
        " perception: %f,"
        " pos: {x:%f, y:%f},"
        " targ: {x:%f, y:%f}"
        " }\n",
        crt->id,
        crt->name,
        CRT_TYPE_NAME(crt->type),
        CRT_STATUS_NAME(crt->status),
        crt->agility,
        crt->w, crt->h,
        crt->perception,
        crt->pos.x, crt->pos.y,
        crt->targ.x, crt->targ.y
    );
}

int crt_draw(Creature *crt, App *app, World *world, SDL_Renderer *renderer, TTF_Font *font) {
    if (!crt || !app || !world || !renderer || !font) {
        return -1;
    }

    int ret;
    float x = crt->pos.x - crt->w / 2;
    float y = crt->pos.y - crt->h / 2;

    // 1. draw pos

    SDL_Rect prect = { (int) x, (int) y, (int) crt->w, (int) crt->h };

    switch (crt->type) {
        case CRT_TYPE_HERBIVORE:
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        break;
        case CRT_TYPE_CARNIVORE:
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        break;
        default:
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }
    ret = SDL_RenderFillRect(renderer, &prect);

    // 2. draw targ;

    if(vec2_equals(crt->pos, crt->targ)) {
        return ret;
    }

    x = crt->targ.x - crt->w / 2;
    y = crt->targ.y - crt->h / 2;

    SDL_Rect trect = { (int) x, (int) y, (int) crt->w, (int) crt->h };
    SDL_SetRenderDrawColor(renderer, 125, 125, 125, 255);
    SDL_RenderDrawLine(renderer, (int) crt->pos.x, (int) crt->pos.y, (int) crt->targ.x, (int) crt->targ.y);
    SDL_RenderDrawRect(renderer, &trect);

    // reset
    // SDL_SetRenderDrawColor(renderer, app->fg_color.r, app->fg_color.g, app->fg_color.b, app->fg_color.a);

    return ret;
}

////
// CrtList
////

CrtList *crt_list_create(size_t max) {
    CrtList *list = malloc(sizeof(CrtList));
    if (!list) {
        return NULL;
    }

    list->len = 0;
    list->grow = max;
    list->max = max;

    list->members = calloc(sizeof(Creature*), max);
    if (!list->members) {
        freez(list);
        return NULL;
    }
    return list;
}

CrtList *crt_list_append(CrtList *list, Creature *crt) {
    if (!list) {
        return NULL;
    }

    if (list->len >= list->max) {
        list->max += list->grow;
        list->members = realloc(list->members, list->max * sizeof(Creature*));
        if (!list->members) {
            freez(list);
            return NULL;
        }
    }
    list->members[list->len] = crt;
    list->len++;

    return list;
}

void crt_list_reset(CrtList *list) {
    if (!list) {
        return;
    }
    for (size_t i = 0; i < list->len; i++) {
        list->members[i] = NULL;
    }
    list->len = 0;
}

void crt_list_destroy(CrtList *list) {
    if (!list) {
        return;
    }
    freez(list->members); // free list, not the nodes
    freez(list);
}

void crt_list_print(FILE *fp, CrtList *list) {
    if(!fp) {
        return;
    }
    if (!list) {
        fprintf(fp, "<NULL>\n");
        return;
    }

    printf(
        "{\n"
        "  len: %ld\n"
        "  max: %ld\n"
        "  grow: %ld\n"
        "  members: [",
        list->len,
        list->max,
        list->grow
    );
    if (list->members) {
        for(size_t i = 0; i < list->len; i++) {
            if (list->members[i]) {
                printf("{id: %d, pos:{%f, %f}}", list->members[i]->id, list->members[i]->pos.x, list->members[i]->pos.y);
            }
            printf("%s", (i < list->len -1) ? ", " : "");
        }
    }
    printf("]\n}\n");
}

////
// Relationships
////

CrtList *crt_find_neighbours(Creature *crt, App *app, World *world, CrtList *list) {
    if (!crt || !app || !world) {
        return NULL;
    }

    if (!list) {
        list = crt_list_create(5);
        EXIT_IF(list == NULL, "failed to allocate memory for CrtList");
    } else {
        crt_list_reset(list);
    }

    Vec2 nw = { crt->pos.x - crt->perception, crt->pos.y - crt->perception };
    Vec2 se = { crt->pos.x + crt->perception, crt->pos.y + crt->perception };
    return qtree_find_in_area(world->qtree, crt, list, nw, se);
}

int crt_draw_neighbours(Creature *crt, CrtList *list, App *app, World *world, SDL_Renderer *renderer, TTF_Font *font){
    if (!crt || !list || !app || !world || !renderer || !font) {
        return -1;
    }

    // 1. draw neighbour area rect
    SDL_SetRenderDrawColor(renderer, 0, 0, 125, 255);
    SDL_Rect rect = {
        (int) (crt->pos.x - crt->perception),
        (int) (crt->pos.y - crt->perception),
        (int) crt->perception*2,
        (int) crt->perception*2
    };
    SDL_RenderDrawRect(renderer, &rect);

    // 4. draw neighbour relationships
    SDL_SetRenderDrawColor(renderer, 125, 0, 0, 255);
    for (size_t i = 0; i < list->len; i++) {
        SDL_RenderDrawLine(renderer, (int) crt->pos.x, (int) crt->pos.y, (int) list->members[i]->pos.x, (int) list->members[i]->pos.y);
    }

    return 0;
}
