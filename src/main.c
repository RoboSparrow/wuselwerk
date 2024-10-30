/**
 * clear && make clean && make && ./wim
 * clear && make clean && make && ./wim -c 1 -f 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getopt

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "app.h"
#include "ui.h"

#include "world.h"
#include "pop.h"
#include "crt.h"

#include "vec2.h"
#include "utils.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600
#define DELAY 3000

#define FONT_PATH "font/UbuntuMono-Regular.ttf"
#define FONT_SZ 12


static void configure(App *app, Population *pop, int argc, char **argv) {
    if (!app || !pop) {
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
                if (ival > POP_MAX) {
                    fprintf(stderr, "invalid '%c' option value: pop > max (%d > %d)\n", opt, ival, POP_MAX);
                    exit(1);
                }
                pop->len = ival;
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

    // parse version info
    ival = app_get_version(app);
    if (ival) {
        LOG_ERROR_F("can't read build info from file '%s'", APP_BUILD_INFO_PATH);
    }
}


int main (int argc, char **argv) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    SDL_Event ev;

    int prev = SDL_GetTicks();

    Population pop = {
        .len=rand_range(1, 10),
        .members={0}
    };

    App app =  {
        .name="Test window",
        .window={
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            DEFAULT_WIDTH, DEFAULT_HEIGHT
        },

        .bg_color={0, 0, 0, SDL_ALPHA_OPAQUE},
        .fg_color={255, 255, 255, SDL_ALPHA_OPAQUE},

        .font_path = FONT_PATH,
        .font_sz = FONT_SZ, // point size for ttf font

        .fps=24, // cinematic film
        .running=1,
        .paused=0,
    };

    configure(&app, &pop, argc, argv);
    window = ui_init_window(&app);
    renderer = ui_init_renderer(&app, window);
    font = ui_init_font(&app);

    Creature *crt;
    CrtType type;
    char name[CRT_NAME_LEN];
    Vec2 pos = {0};

    for (int i = 0; i < pop.len; i++) {
        rand_str(name, CRT_NAME_LEN);
        type = rand_range(1, CRT_TYPE_MAX - 1);
        pos.x = rand_range_f(0, app.window.w);
        pos.y = rand_range_f(0, app.window.h);

        crt = crt_birth(i, name, type, pos);
        crt->agility = rand_range_f(0, 1.f);
        crt_random_targ(crt, &app, 100.f); // TODO radius perception?
        pop.members[i] = crt;

        // crt_print(stdout, crt); // dev
        crt_draw(crt, &app, renderer, font);
    }

    // render changes
    SDL_RenderPresent(renderer);

    while (app.running) {
        while (SDL_PollEvent(&ev) != 0)  {

            if (ev.type == SDL_QUIT) {
                app.running = 0;
                break;
            }

            if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {
                    case SDLK_SPACE:
                        app.paused = !app.paused;
                        // printf(" - app paused: %d\n", app.paused);
                    break;
                }
            }

        }

        if (app.paused) {
            world_update(&app);
            world_draw(&app, renderer, font);
            SDL_RenderPresent(renderer);

            continue;
        }

        if (SDL_GetTicks() - prev < 1000/app.fps) {
            continue;
        }

        SDL_SetRenderDrawColor(renderer, app.bg_color.r, app.bg_color.g, app.bg_color.b, app.bg_color.a);
        SDL_RenderClear(renderer);

        world_update(&app);
        world_draw(&app, renderer, font);

        for (int i = 0; i < pop.len; i++) {
            crt_update(pop.members[i], &app);
            crt_draw(pop.members[i], &app, renderer, font);
        }

        // render changes
        SDL_RenderPresent(renderer);
        prev = SDL_GetTicks();
    } // while

    pop_destroy(&pop);
    ui_exit(&app, window, renderer, font);

  return 0;
}
