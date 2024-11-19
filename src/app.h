#ifndef __APP_H__
#define __APP_H__

#define APP_MAX_FPS 60
#define APP_BUILD_INFO_PATH "./build"

#define APP_STR_LEN 128

#include <GLFW/glfw3.h>

typedef struct App {
    char name[APP_STR_LEN];
    char version[APP_STR_LEN];

    // game state
    size_t fps; // seconds
    int debug;
    int paused;

    // ui
    GLFWwindow *window;
    struct nk_glfw *gui;

    // ui state
    int show_menu;
    int show_crt_info;
} App;

App *app_create(const char *name);
void app_destroy(App *app);

#endif
