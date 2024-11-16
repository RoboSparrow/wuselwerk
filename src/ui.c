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
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"


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
        }
    }
}

static void _gl_error_callback(int err, const char* msg) {
    LOG_ERROR_F("error %d: %s", err, msg);
}

void _gl_resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

GLFWwindow* ui_init(App *app, World *world) {
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

    return window;
}

void ui_exit(GLFWwindow *window) {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

struct nk_glfw *gui_init(GLFWwindow *window) {
    EXIT_IF(window == NULL, "GLFWwindow has not been initialised");

    struct nk_glfw *glfw = calloc(1, sizeof(struct nk_glfw));
    EXIT_IF(glfw == NULL, "failed to allocate memory for struct nk_glfw ");

    struct nk_context *ctx = nk_glfw3_init(glfw, window, NK_GLFW3_INSTALL_CALLBACKS);
    EXIT_IF(ctx == NULL, "error initializing nk context");

    {
        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(glfw, &atlas);
        nk_glfw3_font_stash_end(glfw);
    }


    ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(255, 255, 255, 25)); // = nk_style_item_hide();
    ctx->style.window.border_color = nk_rgba(0,0,0,255);

    return glfw;
}

void gui_exit(struct nk_glfw *gui) {
    if(!gui) {
        return;
    }
    nk_glfw3_shutdown(gui);
    freez(gui);
}

int gui_draw(struct nk_glfw *gui, App *app, World *world) {
    int ret = 0;

    struct nk_context *ctx = &gui->ctx;

    nk_glfw3_new_frame(gui);
    // TODO NK_WINDOW_CLOSABLE
    nk_flags flags = NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE; // declare settings once in App
    nk_bool open = nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250), flags);

    if(!open) {
        // not an error, window could be collapsed
        goto end;
        ret -1;
    }

    struct nk_colorf bg = {.r=0.10f, .g=0.18f, .b=0.24f, .a=1.0f};
    enum {EASY, HARD};
    static int op = EASY;
    static int property = 20;
    nk_layout_row_static(ctx, 30, 80, 1);
    if (nk_button_label(ctx, "button")) {
        fprintf(stdout, "button pressed\n");
    }

    nk_layout_row_dynamic(ctx, 30, 2);
    if (nk_option_label(ctx, "easy", op == EASY)) {
        op = EASY;
    }
    if (nk_option_label(ctx, "hard", op == HARD)) {
        op = HARD;
    }

    nk_layout_row_dynamic(ctx, 25, 1);
    nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

    nk_layout_row_dynamic(ctx, 20, 1);
    nk_label(ctx, "background:", NK_TEXT_LEFT);
    nk_layout_row_dynamic(ctx, 25, 1);
    if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
        nk_layout_row_dynamic(ctx, 120, 1);
        bg = nk_color_picker(ctx, bg, NK_RGBA);
        nk_layout_row_dynamic(ctx, 25, 1);
        bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
        bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
        bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
        bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
        nk_combo_end(ctx);
    }

    end:
        nk_end(ctx);
        nk_glfw3_render(gui, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

    return ret;
}
