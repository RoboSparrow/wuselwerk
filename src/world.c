#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "app.h"
#include "world.h"
#include "crt.h"
#include "ui.h"


void world_print(FILE *fp, World *world) {
    if(!fp) {
        return;
    }
    if (!world) {
        fprintf(fp, "<NULL>\n");
    }

    fprintf(fp, "{ len: %ld, population: [", world->len);
    for (int i = 0; i < world->len; i++) {
        fprintf(fp, "{%d, \"%s\", %s}%s", world->population[i]->id, world->population[i]->name, CRT_TYPE_NAME(world->population[i]->type), (i < world->len - 1) ? ", " : "");
    }
    fprintf(fp, "] }\n");
}

int world_update(App *app) {
    int res = 0;
    // TODO
    return res;
}

int world_draw(App *app, SDL_Renderer *renderer, TTF_Font *font) {
    if (!app || !renderer || !font) {
        return -1;
    }
    int res = 0;

    res = ui_draw_status_bar(app, renderer, font);
    return res;
}

void world_destroy(World *world) {
    for (int i = 0; i < world->len; i++) {
        crt_destroy(world->population[i]);
    }
    world->len = 0;
}
