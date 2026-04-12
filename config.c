#include <stdio.h>
#include <string.h>
#include "game.h"

Config config;

/* load config from a text file, returns 1 on success, 0 on failure */
int load_config(const char *filename) {
    FILE *f;
    char line[256];
    char key[64];
    int value;

    /* defaults config */
    config.cells_w = 13;
    config.cells_h = 7;
    config.seed = 0;       /* 0 = use time */
    config.density = 100;  /* 100 = perfect maze, no walls removed */

    f = fopen(filename, "r");
    if (!f) {
        printf("Config file '%s' not found, using defaults.\n", filename);
        return 0;
    }

    while (fgets(line, sizeof(line), f)) {
        /* skip empty lines and comments */
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
            continue;

        if (sscanf(line, "%63[^=]=%d", key, &value) == 2) {
            if (strcmp(key, "width") == 0) {
                if (value >= 3 && value <= MAX_CELLS_W)
                    config.cells_w = value;
            } else if (strcmp(key, "height") == 0) {
                if (value >= 3 && value <= MAX_CELLS_H)
                    config.cells_h = value;
            } else if (strcmp(key, "seed") == 0) {
                config.seed = (unsigned int)value;
            } else if (strcmp(key, "density") == 0) {
                if (value >= 0 && value <= 100)
                    config.density = value;
            }
        }
    }

    fclose(f);
    return 1;
}
