#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

#include "app.h"
#include "crt.h"
#include "qtree.h" // toto remove
#include "utils.h"
#include "vec2.h"
#include "world.h"

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
    crt->mass = CRT_MIN_MASS;

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

static float _apply_attraction_rules(Creature *crt, Creature *other) {
    switch (crt->type) {

    case CRT_TYPE_HERBIVORE:

        switch (other->type) {
        case CRT_TYPE_CARNIVORE:
            return -.5f; // repel
        case CRT_TYPE_HERBIVORE:
            return 1.5f; // attract
        }

        break;

    case CRT_TYPE_CARNIVORE:

        switch (other->type) {
        case CRT_TYPE_CARNIVORE:
            return 0.f;
        case CRT_TYPE_HERBIVORE:
            return 1.f; // neutral
        }

        break;
    }

    return 1.f; // neutral
}

static int _crt_apply_neighbours(Creature *crt, App *app, World *world, QuadList *neighbours) {
    if (!neighbours->len) {
        return 0;
    }

    Creature *other;
    Vec2 delta, accl;
    float dist, force, attraction, speed;
    int dirx, diry;

    size_t count = 0; // affected
    for (size_t i = 0; i < neighbours->len; i++) {
        other = (Creature *)neighbours->nodes[i]->data;
        if (!other || other->id == crt->id) {
            continue;
        }

        delta = vec2_sub(crt->pos, other->pos);
        dist = vec2_mag(delta);
        if (dist == 0 || dist >= crt->perception) {
            continue;
        }

        // this is a variation of Newton's law of universal gravitation using attraction values instead of gravitation
        attraction = _apply_attraction_rules(crt, other);
        force = attraction * ((crt->mass * other->mass) / (dist * dist));
        // force = GRAVITY * ((crt->mass * other->mass) / (dist * dist));

        // bounds
        dirx = (crt->pos.x < world->nw.x || crt->pos.x > world->se.x) ? 1 : -1;
        diry = (crt->pos.y < world->nw.y || crt->pos.y > world->se.y) ? 1 : -1;

        speed = crt->agility * 25.0; // TODO dynamic

        accl = (Vec2){
            force * delta.x * speed * dirx,
            force * delta.y * speed * diry, // TODO
        };

        // printf("%d(%d) -> %d(%d): force %f, attr: %f, accl { %f, %f }\n", crt->id, crt->type, other->id, other->type, force, attraction, accl.x, accl.y);
        crt->pos.x += accl.x;
        crt->pos.y += accl.y;

        count++;
    }

    return count;
}

/**
 * Main loop: update
 */
int crt_update(Creature *crt, App *app, World *world, QuadList *neighbours) {
    if (!crt || !app || !world) {
        return 1;
    }

    // apply influenc eof neighbouring particles
    int did = _crt_apply_neighbours(crt, app, world, neighbours);
    if (did) {
        return 0;
    }

    // move towards target

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
    if (!fp) {
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
            " mass: %f,"
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
            crt->mass,
            crt->pos.x, crt->pos.y,
            crt->targ.x, crt->targ.y);
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
        glColor4f(1.0, 1.0, 0.0, 1.0);
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

    if (vec2_equals(crt->pos, crt->targ)) {
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
            other = (Creature *)list->nodes[i]->data;
            ohz = other->size / 2;
            if (other->id != crt->id) {
                glBegin(GL_LINES);
                glVertex2f(crt->pos.x - hsz, crt->pos.y - hsz);
                glVertex2f(other->pos.x - ohz, other->pos.y - ohz);
                glEnd();
            }
        }
    }

    return ret;
}
