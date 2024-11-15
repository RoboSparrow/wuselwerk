#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <GLFW/glfw3.h>

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
    crt->size = CRT_MIN_SIZE;

    crt->perception = CRT_MIN_PERCEPTION;

    crt->pos = pos;
    crt->targ = pos;
    return crt;
}

void crt_destroy(Creature *crt) {
    // provision for future complexities
    freez(crt);
}

int crt_random_targ(Creature *crt, World *world, float max_radius) {
    if (!crt || !world) {
        return 1;
    }

    Vec2 delta = vec2_rand_from(crt->pos, max_radius);

    delta = vec2_add(crt->pos, delta);
    delta.x = clamp_f(delta.x, 0, WORLD_WIDTH(world));
    delta.y = clamp_f(delta.y, 0, WORLD_HEIGHT(world));

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
        crt_random_targ(crt, world, 200.f);
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
        " size: %f,"
        " perception: %f,"
        " pos: {x:%f, y:%f},"
        " targ: {x:%f, y:%f}"
        " }\n",
        crt->id,
        crt->name,
        CRT_TYPE_NAME(crt->type),
        CRT_STATUS_NAME(crt->status),
        crt->agility,
        crt->size,
        crt->perception,
        crt->pos.x, crt->pos.y,
        crt->targ.x, crt->targ.y
    );
}

int crt_draw(Creature *crt, App *app, World *world) {
    if (!crt || !app || !world) {
        return -1;
    }

    int ret = 0;
    float hsz = crt->size / 2;
    float x = crt->pos.x - hsz;
    float y = crt->pos.y - hsz;

    // 1. draw pos

    switch (crt->type) {
        case CRT_TYPE_HERBIVORE:
            glColor4f(0.0, 1.0, 0.0, 1.0);
            glPointSize(crt->size);
        break;
        case CRT_TYPE_CARNIVORE:
            glColor4f(0.0, 0.0, 1.0, 1.0);
            glPointSize(crt->size);
        break;
        default:
            glColor4f(1.0, 0.0, 0.0, 1.0);
            glPointSize(crt->size);
    }

    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();

    if (!app->debug) {
        return ret;
    }

    // 2. draw line between pos and targ

    glLineWidth(1.0);
    glColor4f(0.15, 0.15, 0.15, 1.0);

    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(crt->targ.x - 1, crt->targ.y - 1);
    glEnd();

    // 3. draw targ

    if(vec2_equals(crt->pos, crt->targ)) {
        return ret;
    }

    glColor4f(0.7, 0.7, 0.7, 1.0);
    glPointSize(2);

    glBegin(GL_POINTS);
    glVertex2f(crt->targ.x - 1, crt->targ.y - 1);
    glEnd();

    return ret;
}

////
// Relationships
////

QuadList *crt_find_neighbours(Creature *crt, App *app, World *world, QuadList *list) {
    if (!crt || !app || !world) {
        return NULL;
    }

    if (!list) {
        list = qlist_create(5);
        EXIT_IF(list == NULL, "failed to allocate memory for QuadList");
    } else {
        qlist_reset(list);
    }

    return qtree_find_in_area(world->qtree, crt->pos, crt->perception, list);
}

int crt_draw_neighbours(Creature *crt, QuadList *list, App *app, World *world) {
    if (!crt || !list || !app || !world) {
        return -1;
    }

    int ret = 0;
    if (!app->debug) {
        return ret;
    }

    float hsz = crt->size / 2;
    float x = crt->pos.x - hsz;
    float y = crt->pos.y - hsz;

    // 1. draw neighbour area rect
    glColor4f(0.0, 0.0, 0.5, 1.0);
    glLineWidth(1.0);

    glBegin(GL_LINES);
    // top
    glVertex2f(x - crt->perception, y - crt->perception);
    glVertex2f(x + crt->perception, y - crt->perception);
    // right
    glVertex2f(x + crt->perception, y - crt->perception);
    glVertex2f(x + crt->perception, y + crt->perception);
    // bottom
    glVertex2f(x + crt->perception, y + crt->perception);
    glVertex2f(x - crt->perception, y + crt->perception);
    // left
    glVertex2f(x - crt->perception, y + crt->perception);
    glVertex2f(x - crt->perception, y - crt->perception);
    glEnd();

    // 4. draw neighbour relationships
    Creature *other;
    glColor4f(0.5, 0.0, 0.0, 1.0);
    glLineWidth(1.0);

    float ohz;
    for (size_t i = 0; i < list->len; i++) {
        if (list->nodes[i] && list->nodes[i]->data) {
            other = (Creature*) list->nodes[i]->data;
            ohz = other->size / 2;
            if(other->id != crt->id) {
                glBegin(GL_LINES);
                glVertex2f(crt->pos.x - hsz, crt->pos.y - hsz);
                glVertex2f(other->pos.x - ohz, other->pos.y - ohz);
                glEnd();
            }
        }
    }

    return ret;
}
