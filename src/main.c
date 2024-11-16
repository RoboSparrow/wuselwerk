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
    GLFWwindow* window;
    struct nk_glfw *gui;

    // world

    World *world = world_create(
        rand_range(2, 25),
        (Vec2){0},
        (Vec2) {DEFAULT_WIDTH, DEFAULT_HEIGHT}
    );

    // app

    App *app = app_create("Test window");
    app->fps = 24; // cinematic film
    app->debug = 1;
    app->running = 1;
    app->paused = 0;

    configure(app, world, argc, argv);

    // glfw, glew

    window = ui_init(app, world);
    //glfwSetWindowUserPointer (window, app);

    // gui
    gui = gui_init(window);

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
        crt->agility = rand_range_f(0, 1.f);
        crt->perception = (ww > wh) ? wh / 10.f : ww / 10.f;
        crt_random_targ(crt, world, 100.f); // TODO radius perception?
        world->population[i] = crt;
        // crt_print(stdout, crt); // dev
    }

    // main loop

    QuadList *neighbours = qlist_create(5);
    EXIT_IF(neighbours == NULL, "failed to allocate memory for QuadList");

    double now, delta;
    double max = 1.0 / app->fps;
    then = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {

        now = glfwGetTime();
        if(now - then < max) {
            continue;
        }

        if (app->paused) {
            world_update(app, world);
            world_draw(app, world);
            continue;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        world_update(app, world);
        world_draw(app, world);

        for (int i = 0; i < world->len; i++) {
            crt_update(world->population[i], app, world);
            crt_draw(world->population[i], app, world);

            crt_find_neighbours(world->population[i], app, world, neighbours);
            crt_draw_neighbours(world->population[i], neighbours, app, world);
        }

        gui_draw(gui, app, world);

        // render changes
        then = now;

        glEnd();
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // while

    qlist_destroy(neighbours);
    world_destroy(world);
    ui_exit(window);
    app_destroy(app);

  return 0;
}
