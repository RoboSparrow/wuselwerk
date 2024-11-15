#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "utils.h"

/**
 * Parses build version info fomr "bild" file.
 * This file has only one line with the version value
 */
static int _get_version(App *app) {
    FILE *fp = fopen(APP_BUILD_INFO_PATH, "r");
    if (!fp) {
        return -1;
    }

    char *res = fgets(app->version, APP_STR_LEN, fp);
    if(!res){
        app->version[0] = 0;
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

App *app_create(const char *name) {
    App *app = calloc(1, sizeof(App));
    EXIT_IF(app == NULL, "error allocating memory for app");

    // set name
    strncpy(app->name, name, APP_STR_LEN);

    // load parse version info
    int res = _get_version(app);
    if (res) {
        LOG_ERROR_F("can't read build info from file '%s'", APP_BUILD_INFO_PATH);
    }

    return app;
}

void app_destroy(App *app) {
    if (!app) {
        return;
    }
    freez(app);
}
