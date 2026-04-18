#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "game.h"

Config config;

static char *skip_spaces(char *text) { /*skips spaces and increments pointer along to non-whitespace*/
    while (*text != '\0' && isspace((unsigned char)*text))
        text++;
    return text;
}

static void trim_end(char *text) { /*checks for whitespace at the end of the string, replaces with null terminator, decrement position pointer*/
    size_t length = strlen(text);

    while (length > 0 && isspace((unsigned char)text[length - 1])) {
        text[length - 1] = '\0';
        length--;
    }
}

static int parse_int_value(const char *text, int *out_value) { /* ensures value is valid numerical, otherwise force failure*/
    char *endptr;
    long parsed;

    parsed = strtol(text, &endptr, 10); /* no base 10 digits were consumed, parsing fails*/
    if (text == endptr)
        return 0;

    while (*endptr != '\0' && isspace((unsigned char)*endptr)) /*skip whitespace, increment pointer*/
        endptr++;

    if (*endptr != '\0')
        return 0;

    *out_value = (int)parsed;
    return 1;
}

/* load config from a text file, returns 1 on success, 0 on failure */
int load_config(const char *filename) {
    FILE *f; /* file handle */
    char line[256]; /* buffer for a single line*/
    int line_number;
    int had_error;
    int saw_width;
    int saw_height;
    int saw_seed;
    int saw_density;
    Config parsed_config;

    /* defaults config */
    config.cells_w = 13;
    config.cells_h = 7;
    config.seed = 0;       /* 0 = use time */
    config.density = 100;  /* 100 = perfect maze, no walls removed */
    parsed_config = config;
    line_number = 0;
    had_error = 0;
    /*duplicate seen booleans*/
    saw_width = 0;
    saw_height = 0;
    saw_seed = 0;
    saw_density = 0;

    f = fopen(filename, "r");
    if (!f) { /*missing file guard*/
        printf("Config file '%s' not found, using defaults.\n", filename);
        return 0;
    }

    while (fgets(line, sizeof(line), f)) {
        char *content;
        char *equals_sign;
        char *key;
        char *value_text;
        int value; /* store value*/

        line_number++;
        content = skip_spaces(line);

        /* skip empty lines and comments */
        if (*content == '#' || *content == '\n' || *content == '\r' || *content == '\0')
            continue;

        trim_end(content);
        equals_sign = strchr(content, '='); /* find position of =*/
        if (equals_sign == NULL) {
            printf("Config parse error in '%s' line %d: missing '='.\n", filename, line_number);
            had_error = 1;
            continue;
        }

        if (strchr(equals_sign + 1, '=') != NULL) { /*check rest of string after first = for more than 1*/
            printf("Config parse error in '%s' line %d: too many '=' characters.\n", filename, line_number);
            had_error = 1;
            continue;
        }

        *equals_sign = '\0'; /* break string into 2 strings, key and value with their own null terminators*/
        key = skip_spaces(content); /*start from left side of string*/
        trim_end(key);
        value_text = skip_spaces(equals_sign + 1); /*start from right of previous =*/
        trim_end(value_text);

        if (*key == '\0') { /*if pointer to start of 1st part is null, key is just empty*/
            printf("Config parse error in '%s' line %d: missing key before '='.\n", filename, line_number);
            had_error = 1;
            continue;
        }

        if (*value_text == '\0') { /*if pointer to start of 2nd part is null terminator, value is empty*/
            printf("Config parse error in '%s' line %d: missing value for '%s'.\n",
                   filename, line_number, key);
            had_error = 1;
            continue;
        }

        if (!parse_int_value(value_text, &value)) {
            printf("Config parse error in '%s' line %d: invalid integer '%s' for '%s'.\n",
                   filename, line_number, value_text, key);
            had_error = 1;
            continue;
        }

        if (strcmp(key, "width") == 0) {
            if (saw_width) { /*already saw a width key earlier, this is dupe*/
                printf("Config parse error in '%s' line %d: duplicate key 'width'.\n",
                       filename, line_number);
                had_error = 1;
                continue;
            }
            saw_width = 1;
            if (value < 3 || value > MAX_CELLS_W) {
                printf("Config parse error in '%s' line %d: width must be between 3 and %d.\n",
                       filename, line_number, MAX_CELLS_W);
                had_error = 1;
                continue;
            }
            parsed_config.cells_w = value;
        } else if (strcmp(key, "height") == 0) {
            if (saw_height) {
                printf("Config parse error in '%s' line %d: duplicate key 'height'.\n",
                       filename, line_number);
                had_error = 1;
                continue;
            }
            saw_height = 1;
            if (value < 3 || value > MAX_CELLS_H) {
                printf("Config parse error in '%s' line %d: height must be between 3 and %d.\n",
                       filename, line_number, MAX_CELLS_H);
                had_error = 1;
                continue;
            }
            parsed_config.cells_h = value;
        } else if (strcmp(key, "seed") == 0) {
            if (saw_seed) {
                printf("Config parse error in '%s' line %d: duplicate key 'seed'.\n",
                       filename, line_number);
                had_error = 1;
                continue;
            }
            saw_seed = 1;
            if (value < 0) {
                printf("Config parse error in '%s' line %d: seed must be zero or greater.\n",
                       filename, line_number);
                had_error = 1;
                continue;
            }
            parsed_config.seed = (unsigned int)value;
        } else if (strcmp(key, "density") == 0) {
            if (saw_density) {
                printf("Config parse error in '%s' line %d: duplicate key 'density'.\n",
                       filename, line_number);
                had_error = 1;
                continue;
            }
            saw_density = 1;
            if (value < 0 || value > 100) {
                printf("Config parse error in '%s' line %d: density must be between 0 and 100.\n",
                       filename, line_number);
                had_error = 1;
                continue;
            }
            parsed_config.density = value;
        } else {
            printf("Config parse error in '%s' line %d: unknown key '%s'.\n",
                   filename, line_number, key);
            had_error = 1;
        }
    }

    fclose(f);
    if (had_error) {
        printf("Config file '%s' is invalid, using defaults.\n", filename);
        return 0;
    }

    config = parsed_config;
    return 1;
}
