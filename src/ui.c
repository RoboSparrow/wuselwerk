#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getopt

#include <GLFW/glfw3.h>

#include "app.h"
#include "world.h"
#include "utils.h"
#include "ui.h"


GLFWwindow* ui_init_window(App *app, World *world) {
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

    return window;
}

void ui_exit(App *app, GLFWwindow *window) {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}
