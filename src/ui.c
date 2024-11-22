#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getopt

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "external/nuklear.h"
#include "nk_glfw3.h"

#include "app.h"
#include "crt.h"
#include "ui.h"
#include "utils.h"
#include "world.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

static void _draw_menu_toggle(App *app, struct nk_glfw *gui, struct nk_context *ctx);
static void _draw_menu(App *app, struct nk_glfw *gui, struct nk_context *ctx, World *world);
static void _draw_crt_info(App *app, struct nk_glfw *gui, struct nk_context *ctx, World *world);

// TODO mv to nk_glfw3.h
struct nk_canvas {
    struct nk_command_buffer *painter;
    struct nk_vec2 item_spacing;
    struct nk_vec2 panel_padding;
    struct nk_style_item window_background;
};

static void _nk_canvas_begin(
    const char *name,
    struct nk_context *ctx, struct nk_canvas *canvas, nk_flags flags,
    int x, int y, int width, int height, struct nk_color background_color);

static void _nk_canvas_end(struct nk_context *ctx, struct nk_canvas *canvas);

/**
 * key input callback
 * @see https://www.glfw.org/docs/latest/input_guide.html#input_key
 * @see https://www.glfw.org/docs/latest/group__keys.html
 */
static void _gl_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    App *app = (App *)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            LOG_INFO("Closing app");
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_D:
            LOG_INFO("toggling debug mode");
            if (app) {
                app->show_targ = !app->show_targ;
                app->show_crt_info = app->show_targ;
                app->show_neighbours = app->show_targ;
            }
            break;
        case GLFW_KEY_SPACE:
            LOG_INFO("toggling pause");
            if (app) {
                app->paused = !app->paused;
            }
            break;
        }
    }
}

static void _gl_error_callback(int err, const char *msg) {
    LOG_ERROR_F("error %d: %s", err, msg);
}

void _gl_resize_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void ui_init(App *app, World *world) {
    EXIT_IF(app == NULL, "App has not been initialised");
    EXIT_IF(world == NULL, "World has not been initialised");

    glfwSetErrorCallback(_gl_error_callback);

    int res = glfwInit();
    EXIT_IF(res == 0, "GFLW failed to initialise.");

    GLFWwindow *window = glfwCreateWindow(
        (int)WORLD_WIDTH(world),
        (int)WORLD_HEIGHT(world),
        app->name,
        NULL, NULL);
    if (!window) {
        glfwTerminate();
        LOG_ERROR("GLFW window failed to initialise");
        exit(EXIT_FAILURE);
    }

    glfwSetFramebufferSizeCallback(window, _gl_resize_callback);
    glfwSetKeyCallback(window, _gl_key_callback);
    glfwMakeContextCurrent(window);

    int fw, fh;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glfwGetFramebufferSize(window, &fw, &fh);
    glOrtho(0.0f, fw, fh, 0.0f, 0.0f, 1.0f);

    glewExperimental = 1;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to setup GLEW\n");
        exit(1);
    }

    app->window = window;
}

void ui_exit(GLFWwindow *window) {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void gui_init(App *app) {
    EXIT_IF(app == NULL, "App has not been initialised");
    EXIT_IF(app->window == NULL, "GLFWwindow has not been initialised");

    struct nk_glfw *glfw = calloc(1, sizeof(struct nk_glfw));
    EXIT_IF(glfw == NULL, "failed to allocate memory for struct nk_glfw ");

    app->gui = glfw;

    struct nk_context *ctx = nk_glfw3_init(app, NK_GLFW3_INSTALL_CALLBACKS);
    EXIT_IF(ctx == NULL, "error initializing nk context");

    {
        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(glfw, &atlas);
        nk_glfw3_font_stash_end(glfw);
    }

    ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(255, 255, 255, 25)); // = nk_style_item_hide();
    ctx->style.window.border_color = nk_rgba(0, 0, 0, 255);
}

void gui_exit(struct nk_glfw *gui) {
    if (!gui) {
        return;
    }
    nk_glfw3_shutdown(gui);
    freez(gui);
}

int gui_draw(App *app, World *world) {
    if (!app || !world || !app->gui) {
        LOG_ERROR("missing app, world or gui");
        return -1;
    }

    int ret = 0;

    struct nk_glfw *gui = app->gui;
    struct nk_context *ctx = &gui->ctx;

    nk_glfw3_new_frame(app->gui);

    if (app->show_crt_info) {
        _draw_crt_info(app, gui, ctx, world);
    }

    _draw_menu_toggle(app, gui, ctx);
    if (app->show_menu) {
        _draw_menu(app, gui, ctx, world);
    }

    nk_glfw3_render(app->gui, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

    return ret;
}

static void _draw_menu(App *app, struct nk_glfw *gui, struct nk_context *ctx, World *world) {
    // params tested before
    int w = 550;
    int h = 250;
    char msg[256];
    char sval[16];
    Rule *rule;

    // TODO
    nk_flags flags = NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE; // declare settings once in App
    nk_bool open = nk_begin(ctx, "Menu", nk_rect((gui->display_width - w) / 2, (gui->display_height - h) / 2, w, h), flags);

    if (!open) {
        // not an error, window could be collapsed
        nk_end(ctx);
        return;
    }

    nk_layout_row_dynamic(ctx, 20, 1);
    nk_checkbox_label(ctx, "crt info", &app->show_crt_info);
    nk_checkbox_label(ctx, "target", &app->show_targ);
    nk_checkbox_label(ctx, "show quads", &app->show_quads);
    nk_checkbox_label(ctx, "show neighbours", &app->show_neighbours);
    nk_checkbox_label(ctx, "show perception", &app->show_perception);

    snprintf(msg, 256, "max fps: %ld", app->fps);
    nk_label(ctx, msg, NK_TEXT_LEFT);

    snprintf(msg, 256, "version: %s", app->version);
    nk_label(ctx, msg, NK_TEXT_LEFT);

    if (world->rules) {
        for (size_t i = 0; i < world->rules->len; i++) {
            rule = world->rules->rules[i];
            if (rule) {
                snprintf(msg, 256, "%s -> %s: %f", crt_type_names[rule->left], crt_type_names[rule->right], rule->val);
                nk_label(ctx, msg, NK_TEXT_LEFT);
                nk_slider_float(ctx, -10.0f, &rule->val, 10.0f, 0.1f);
            }
        }
    }
    nk_end(ctx);
}

static void _draw_menu_toggle(App *app, struct nk_glfw *gui, struct nk_context *ctx) {
    // params tested before
    int w = 100;
    int h = 30;

    nk_flags flags = NK_WINDOW_NO_SCROLLBAR;
    nk_bool open = nk_begin(ctx, "Menu Toggle", nk_rect(0, gui->display_height - h, w, h), flags);

    if (!open) {
        return;
    }

    nk_layout_row_dynamic(ctx, h - 10, 1);
    nk_checkbox_label(ctx, (app->show_menu) ? "Close Menu" : "Open menu", &app->show_menu);

    nk_end(ctx);
}

static void _draw_crt_info(App *app, struct nk_glfw *gui, struct nk_context *ctx, World *world) {
    if (world->len <= 0) {
        return;
    }

    struct nk_canvas canvas;
    struct nk_font *font = gui->atlas.default_font;
    struct nk_color bg = nk_rgba(255, 0, 0, 0);
    struct nk_color fg = nk_rgba(255, 255, 255, 255);
    struct nk_rect rect = nk_rect(0, 0, 150, 20);

    char msg[128];

    _nk_canvas_begin("crt info", ctx, &canvas, NK_WINDOW_BACKGROUND, 0, 0, gui->display_width, gui->display_height, bg);

    for (size_t i = 0; i < world->len; i++) {
        if (!world->population[i]) {
            continue;
        }

        rect = nk_rect(
            world->population[i]->pos.x - (world->population[i]->size / 2),
            world->population[i]->pos.y - (world->population[i]->size / 2) - 25,
            50, 20);
        snprintf(msg, 128, "%d", world->population[i]->id);
        nk_draw_text(canvas.painter, rect, msg, strlen(msg), &font->handle, bg, fg);
    }

    _nk_canvas_end(ctx, &canvas);
}

static void _nk_canvas_begin(
    const char *name,
    struct nk_context *ctx, struct nk_canvas *canvas, nk_flags flags,
    int x, int y, int width, int height, struct nk_color background_color) {

    //save style properties which will be overwritten
    canvas->panel_padding = ctx->style.window.padding;
    canvas->item_spacing = ctx->style.window.spacing;
    canvas->window_background = ctx->style.window.fixed_background;

    // use the complete window space and set background
    ctx->style.window.spacing = nk_vec2(0, 0);
    ctx->style.window.padding = nk_vec2(0, 0);
    ctx->style.window.fixed_background = nk_style_item_color(background_color);

    // create/update window and set position + size
    flags = flags & ~NK_WINDOW_DYNAMIC;
    nk_window_set_bounds(ctx, name, nk_rect(x, y, width, height));
    nk_begin(ctx, name, nk_rect(x, y, width, height), NK_WINDOW_NO_SCROLLBAR | flags);

    // allocate the complete window space for drawing
    {
        struct nk_rect total_space;
        total_space = nk_window_get_content_region(ctx);
        nk_layout_row_dynamic(ctx, total_space.h, 1);
        nk_widget(&total_space, ctx);
        canvas->painter = nk_window_get_canvas(ctx);
    }
}

static void _nk_canvas_end(struct nk_context *ctx, struct nk_canvas *canvas) {
    nk_end(ctx);
    ctx->style.window.spacing = canvas->panel_padding;
    ctx->style.window.padding = canvas->item_spacing;
    ctx->style.window.fixed_background = canvas->window_background;
}
