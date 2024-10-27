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
        return -1;
    }

    fclose(fp);
    return 0;
}
