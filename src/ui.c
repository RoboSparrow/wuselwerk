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
#include "world.h"
#include "utils.h"
#include "ui.h"


#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

/**
 * key input callback
 * @see https://www.glfw.org/docs/latest/input_guide.html#input_key
 * @see https://www.glfw.org/docs/latest/group__keys.html
 */
static void _gl_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    App *app = (App*) glfwGetWindowUserPointer(window);

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
                    app->debug = !app->debug;
                }
            break;
            case GLFW_KEY_SPACE :
                LOG_INFO("toggling pause");
                if (app) {
                    app->paused = !app->paused;
                }
            break;
        }
    }
}

static void _gl_error_callback(int err, const char* msg) {
    LOG_ERROR_F("error %d: %s", err, msg);
}

void _gl_resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void ui_init(App *app, World *world) {
    EXIT_IF(app == NULL, "App has not been initialised");
    EXIT_IF(world == NULL, "World has not been initialised");

    glfwSetErrorCallback(_gl_error_callback);

    int res = glfwInit();
    EXIT_IF(res == 0, "GFLW failed to initialise.");

    GLFWwindow *window = glfwCreateWindow(
        (int) WORLD_WIDTH(world),
        (int) WORLD_HEIGHT(world),
         app->name,
         NULL, NULL
    );
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
    glOrtho(0, fw, 0, fh, -1, 1);

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
    ctx->style.window.border_color = nk_rgba(0,0,0,255);
    ctx->style.checkbox.active = nk_style_item_color(nk_rgb(255, 0, 0));

}

void gui_exit(struct nk_glfw *gui) {
    if(!gui) {
        return;
    }
    nk_glfw3_shutdown(gui);
    freez(gui);
}

int gui_draw(App *app, World *world) {
    if(!app || !world || !app->gui) {
        LOG_ERROR("missing app, world or gui");
        return -1;
    }

    int ret = 0;
    char msg[256];

    struct nk_context *ctx = &app->gui->ctx;

    nk_glfw3_new_frame(app->gui);
    // TODO NK_WINDOW_CLOSABLE
    nk_flags flags = NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE; // declare settings once in App
    nk_bool open = nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250), flags);

    if(!open) {
        // not an error, window could be collapsed
        goto end;
        ret -1;
    }

    nk_layout_row_dynamic(ctx, 20, 1);
    nk_checkbox_label(ctx, "debug", &app->debug);

    snprintf(msg, 256, "max fps: %ld", app->fps);
    nk_label(ctx, msg, NK_TEXT_LEFT);

    snprintf(msg, 256, "version: %s", app->version);
    nk_label(ctx, msg, NK_TEXT_LEFT);

    end:
        nk_end(ctx);
        nk_glfw3_render(app->gui, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

    return ret;
}
