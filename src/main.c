/**
 * clear && make clean && make && ./wusel
 * clear && make clean && make && ./wusel -c 1 -f 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getopt

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "app.h"
#include "ui.h"

#include "world.h"
#include "crt.h"

#include "vec2.h"
#include "utils.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600
#define DELAY 3000

#define FONT_PATH "font/UbuntuMono-Regular.ttf"
#define FONT_SZ 12

double then;

static void configure(App *app, World *world, int argc, char **argv) {
    if (!app || !world) {
        return;
    }

    int opt;
    int ival;

    char usage[] = "usage: %s [-h] [-c creatures:number] [-P paused]\n";
    while ((opt = getopt(argc, argv, "f:c:Ph")) != -1) {
        switch (opt)  {
            case 'c':
                ival = atoi(optarg);
                if (!ival || ival < 0) {
                    fprintf(stderr, "invalid '%c' option value\n", opt);
                    exit(1);
                }
                if (ival > WORLD_POP_MAX) {
                    fprintf(stderr, "invalid '%c' option value: pop > max (%d > %d)\n", opt, ival, WORLD_POP_MAX);
                    exit(1);
                }
                world->len = ival;
            break;

            case 'f':
                ival = atoi(optarg);
                if (!ival || ival < 0) {
                    fprintf(stderr, "invalid 'f' option value\n");
                    exit(1);
                }
                if (ival > APP_MAX_FPS) {
                    fprintf(stderr, "invalid '%c' option value: fps > max (%d > %d)\n", opt, ival, APP_MAX_FPS);
                    exit(1);
                }
                app->fps = ival;
            break;

            case 'P':
                app->paused = 1;
            break;

            case 'h':
            case '?':
                fprintf(stderr, usage, argv[0]);
                exit(0);
            break;
        }
    }
}

int main (int argc, char **argv) {

    // world

    World *world = world_create(
        rand_range(15, 200),
        (Vec2){0},
        (Vec2) {DEFAULT_WIDTH, DEFAULT_HEIGHT}
    );

    // app

    App *app = app_create("Test window");
    app->fps = 24; // cinematic film
    app->debug = 0;
    app->paused = 0;

    configure(app, world, argc, argv);

    // glfw, glew, gui
    ui_init(app, world);
    gui_init(app);

    // store up for callback updates
    glfwSetWindowUserPointer (app->window, app);

    float ww = WORLD_WIDTH(world);
    float wh = WORLD_HEIGHT(world);

    // population

    Creature *crt;
    CrtType type;
    char name[CRT_NAME_LEN];
    Vec2 pos = {0};

    for (int i = 0; i < world->len; i++) {
        rand_str(name, CRT_NAME_LEN);
        type = rand_range(1, CRT_TYPE_MAX - 1);
        pos.x = rand_range_f(0, ww);
        pos.y = rand_range_f(0, wh);

        crt = crt_birth(i, name, type, pos);
        crt->mass = rand_range_f(CRT_MIN_MASS, 5.f);
        crt->size = crt->mass;
        crt->agility = rand_range_f(.1, 2.f) * (1 / (crt->mass + crt->size)); // inverse proportional to mass (rand_range_f(.1, 2.f));

        // printf(" -%d(%d): m: %f, s: %f, a: %f\n", crt->id, crt->type, crt->mass, crt->size, crt->agility);

        crt->perception = (ww > wh) ? wh / 10.f : ww / 10.f;
        crt_random_targ(crt, world, 100.f);
        world->population[i] = crt;
    }

    // main loop

    QuadList *neighbours = qlist_create(5);
    EXIT_IF(neighbours == NULL, "failed to allocate memory for QuadList");

    double now, delta;
    double max = 1.0 / app->fps;
    then = glfwGetTime();

    while (!glfwWindowShouldClose(app->window)) {

        now = glfwGetTime();
        if(now - then < max) {
            continue;
        }

        if (app->paused) {
            world_update(app, world);
            world_draw(app, world);
        } else {
            glClear(GL_COLOR_BUFFER_BIT);

            world_update(app, world);
            world_draw(app, world);

            for (int i = 0; i < world->len; i++) {
                crt_find_neighbours(world->population[i], app, world, neighbours);
                crt_update(world->population[i], app, world, neighbours);

                crt_draw(world->population[i], app, world);
                crt_draw_neighbours(world->population[i], neighbours, app, world);
            }

            gui_draw(app, world);

            // render changes
            then = now;
        }

        glEnd();
        glfwSwapBuffers(app->window);
        glfwPollEvents();

    } // while

    qlist_destroy(neighbours);
    world_destroy(world);
    ui_exit(app->window);
    gui_exit(app->gui);
    app_destroy(app);

  return 0;
}
