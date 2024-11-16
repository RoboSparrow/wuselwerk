#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getopt

#include <GLFW/glfw3.h>

#include "app.h"
#include "world.h"
#include "utils.h"
#include "ui.h"

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

    return window;
}

void ui_exit(GLFWwindow *window) {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}
