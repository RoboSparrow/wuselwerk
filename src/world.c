#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

#include "app.h"
#include "world.h"
#include "qtree.h"
#include "crt.h"
#include "ui.h"
#include "utils.h"

World *world_create(size_t len, Vec2 nw, Vec2 se) {
    World *world = calloc(sizeof(World), 1);
    EXIT_IF(world == NULL, "failed to allocate memory for world(1)");

    // dimensions
    world->nw = nw;
    world->se = se;

    // population
    world->len = len;
    for (size_t i = 0; i < WORLD_POP_MAX; i++) {
        world->population[i] = NULL; // dumb fill until we have a memory managed population array (TODO)
    }
    return world;
}

void world_print(FILE *fp, World *world) {
    if(!fp) {
        return;
    }

    if (!world) {
        fprintf(fp, "<NULL>\n");
    }
    Creature *crt;

    fprintf(fp,
        "{\n"
        "  nw: {%.2f, %.2f},\n"
        "  se: {%.2f, %.2f},\n"
        "  len: %ld,\n",
        world->nw.x, world->nw.y,
        world->se.x, world->se.y,
        world->len
    );

    fprintf(fp, "  population: [");
    for (int i = 0; i < world->len; i++) {
        crt = world->population[i];
        if (!crt) {
            fprintf(fp, "NULL");
        } else {
            fprintf(fp, "{%d, \"%s\", %s}", crt->id, crt->name, CRT_TYPE_NAME(crt->type));
        }
        fprintf(fp, "%s", (i < world->len - 1) ? ", " : "");
    }
    fprintf(fp, "],\n");

    fprintf(fp, "  qtree: ");
    qtree_print(fp, world->qtree);
    fprintf(fp, "\n}\n");
}

void world_destroy(World *world) {
    if (!world) {
        return;
    }

    // population
    if (world->population) {
        for (int i = 0; i < world->len; i++) {
            crt_destroy(world->population[i]);
        }
    }
    world->len = 0;

    // quad tree
    qtree_destroy(world->qtree);

    // world
    freez(world);
}

/**
 * Main loop: update
 */
int world_update(App *app, World *world) {
    if (!app || !world) {
        return -1;
    }

    // 1. rebuild quad tree

    if (world->population) {

        qtree_destroy(world->qtree); // TODO optimize (pre-alloc MAX_POP)
        world->qtree = qtree_create(world->nw, world->se);
        EXIT_IF(world->qtree == NULL, "failed to allocate memory for world tree");

        for (int i = 0; i < world->len; i++) {
            if (world->population[i]) {
                qtree_insert(world->qtree, world->population[i], world->population[i]->pos);
            }
        }
    }

    // TODO
    return 0;
}

static void _draw_qtree_asc(QuadNode *node) {
    if(!node) {
        return;
    }

    glLineWidth(1.0);
    glColor4f(0.15, 0.15, 0.15, 1.0);

    // right
    glBegin(GL_LINES);
    glVertex2f(node->self_se.x, node->self_nw.y);
    glVertex2f(node->self_se.x, node->self_se.y);
    glEnd();

    //  bottom
    glBegin(GL_LINES);
    glVertex2f(node->self_nw.x, node->self_se.y);
    glVertex2f( node->self_se.x, node->self_se.y);
    glEnd();

}
static void _draw_qtree_desc(QuadNode *node) {}

/**
 * Main loop: draw
 */
int world_draw(App *app, World *world) {
    if (!app || !world) {
        return -1;
    }

    int res = 0;

    // draw quads
    if (world->qtree) {
        qnode_walk(world->qtree->root, _draw_qtree_asc, _draw_qtree_desc);
    }

    return res;
}
