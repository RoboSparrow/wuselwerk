#ifndef __APP_H__
#define __APP_H__

#define APP_MAX_FPS 60
#define APP_BUILD_INFO_PATH "./build"

#define APP_STR_LEN 128

typedef struct App {
    char name[APP_STR_LEN];
    char version[APP_STR_LEN];

    size_t fps; // seconds
    size_t debug;

    char running;
    char paused;
} App;

App *app_create(const char *name);
void app_destroy(App *app);

#endif
