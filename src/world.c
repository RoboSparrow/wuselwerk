#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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

SDL_Renderer *_renderer; // TODO
static void _draw_qtree_asc(QuadNode *node) {
    if(!node) {
        return;
    }
    // right
    SDL_RenderDrawLine(_renderer, node->self_se.x, node->self_nw.y, node->self_se.x, node->self_se.y);
    // bottom x
    SDL_RenderDrawLine(_renderer, node->self_nw.x, node->self_se.y, node->self_se.x, node->self_se.y);
}
static void _draw_qtree_desc(QuadNode *node) {}

/**
 * Main loop: draw
 */
int world_draw(App *app, World *world, SDL_Renderer *renderer, TTF_Font *font) {
    if (!app || !world || !renderer || !font) {
        return -1;
    }

    int res = 0;

    // draw quads
    if (world->qtree) {
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        _renderer = renderer;
        qnode_walk(world->qtree->root, _draw_qtree_asc, _draw_qtree_desc);
    }
    res = ui_draw_status_bar(app, world, renderer, font);

    return res;
}
