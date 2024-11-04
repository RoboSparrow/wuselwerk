#include <stdio.h>
#include <stdlib.h>

#include "app.h"
#include "utils.h"

/**
 * Parses build version info fomr "bild" file.
 * This file has only one line with the version value
 */
int app_get_version(App *app) {
    FILE *fp = fopen(APP_BUILD_INFO_PATH, "r");
    if (!fp) {
        return -1;
    }

    char *res = fgets(app->version, sizeof(app->version), fp);
    if(!res){
        app->version[0] = 0;
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

void app_destroy(App *app) {
    if (!app) {
        return;
    }
    // TODO placeholder for planned allocation
}
