#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "game.h"

#define SAVE_MAGIC "ABYSS2" /* file signature to detect valid save files*/
#define SAVE_MAGIC_SIZE 6 /* file signature size*/

static int write_u32(FILE *fp, unsigned long value) {
    unsigned char bytes[4]; /* array of 4 bytes*/

    if (value > 0xFFFFFFFFUL) /* max value in 32 bytes, any bigger value fails gracefully*/
        return 0;

    bytes[0] = (unsigned char)(value & 0xFFUL); /* bit mask to extract each byte, then shift rightwards by 8 bits, force little endianness, MSB at higher address*/
    bytes[1] = (unsigned char)((value >> 8) & 0xFFUL);
    bytes[2] = (unsigned char)((value >> 16) & 0xFFUL);
    bytes[3] = (unsigned char)((value >> 24) & 0xFFUL);
    return fwrite(bytes, sizeof(unsigned char), 4, fp) == 4;
}

static int read_u32(FILE *fp, unsigned long *value) {
    unsigned char bytes[4]; /* destination array of 4 bytes */

    if (fread(bytes, sizeof(unsigned char), 4, fp) != 4) /* each item is 1 unsigned char, 1 byte*/
        return 0; /* if fail to read 4 bytes, fail gracefully*/

    *value = (unsigned long)bytes[0]
           | ((unsigned long)bytes[1] << 8)
           | ((unsigned long)bytes[2] << 16)
           | ((unsigned long)bytes[3] << 24);
    return 1;
}

static int write_entity(FILE *fp, const Entity *entity) {
    unsigned char symbol; /* 1 byte for symbol*/

    if (entity->x < 0 || entity->y < 0) /* reject negative coordiantes gracefully*/
        return 0;

    symbol = (unsigned char)entity->symbol;
    return write_u32(fp, (unsigned long)entity->x) /* write x coordinate in little endian, chain the operations*/
        && write_u32(fp, (unsigned long)entity->y)
        && fwrite(&symbol, sizeof(unsigned char), 1, fp) == 1; /* write property by property instead of entire entity stuct in one go that is dependent on compiler struct padding*/
}

static int read_entity(FILE *fp, Entity *entity) {
    unsigned long x;
    unsigned long y;
    unsigned char symbol;

    if (!read_u32(fp, &x) || !read_u32(fp, &y) /* read bytes for x and y*/
        || fread(&symbol, sizeof(unsigned char), 1, fp) != 1 /* read single byte for symbol*/
        || x > (unsigned long)INT_MAX || y > (unsigned long)INT_MAX) { /* check if cooridnates are too big*/
        return 0;
    }

    entity->x = (int)x;
    entity->y = (int)y;
    entity->symbol = (char)symbol;
    return 1;
}

static int is_valid_floor_position(int x, int y) { /* return 1 if position is valid, 0 otherwise*/
    return x >= 0 && x < map_width && y >= 0 && y < map_height
        && cave_map[y][x] == ' ';
}

static int validate_loaded_state(int steps) {
    int x, y;

    if (steps < 0 || has_weapon < 0 || has_weapon > 1
        || monster_dead < 0 || monster_dead > 1) { /* reject invalid game states*/
        return 0;
    }

    for (y = 0; y < MAX_MAP_HEIGHT; y++) { /* check all map cells for invalid characters*/
        for (x = 0; x < MAX_MAP_WIDTH; x++) {
            if (cave_map[y][x] != ' ' && cave_map[y][x] != '#')
                return 0;
        }
    }

    if (!is_valid_floor_position(player.x, player.y)
        || !is_valid_floor_position(enemy.x, enemy.y)
        || !is_valid_floor_position(weapon.x, weapon.y)) {
        return 0;
    }

    if (player.symbol != '@' || enemy.symbol != 'M' || weapon.symbol != 'B')
        return 0;

    return 1;
}

void save_game(int steps) {
    FILE *fp = fopen("maze.sav", "wb");
    if (fp == NULL) { /* gracefull failure if unable to read file*/
        printf("\n>>> ERROR: Could not create save file! <<<\n");
        return;
    }

    /* write a small file signature so load can reject invalid or old saves safely */
    if (fwrite(SAVE_MAGIC, sizeof(char), SAVE_MAGIC_SIZE, fp) != SAVE_MAGIC_SIZE
        || !write_u32(fp, (unsigned long)map_width) /* write map dimensions as 4 byte integers in little endian */
        || !write_u32(fp, (unsigned long)map_height)
        || fwrite(cave_map, sizeof(char), MAX_MAP_HEIGHT * MAX_MAP_WIDTH, fp) != MAX_MAP_HEIGHT * MAX_MAP_WIDTH /* ptr to destination, size of 1 item, number of items, stream to write into. Then != check if exactly height x width items were written*/
        || !write_entity(fp, &player)
        || !write_entity(fp, &enemy)
        || !write_entity(fp, &weapon)
        || !write_u32(fp, (unsigned long)has_weapon)
        || !write_u32(fp, (unsigned long)monster_dead)
        || !write_u32(fp, (unsigned long)steps)) { /* if any write operation fails, exit gracefully*/
        fclose(fp);
        printf("\n>>> ERROR: Failed to write save file completely! <<<\n");
        return;
    }

    fclose(fp);
    printf("\n>>> GAME SAVED SUCCESSFULLY TO maze.sav <<<\n");
}

/* returns the loaded step count, or -1 if the file doesnt exist */
int load_game(void) {
    int loaded_steps;
    char magic[SAVE_MAGIC_SIZE];
    unsigned long width_value;
    unsigned long height_value;
    unsigned long flag_value;
    unsigned long step_value;

    FILE *fp = fopen("maze.sav", "rb");
    if (fp == NULL) {
        printf("\n>>> ERROR: No save file found! <<<\n");
        return -1;
    }

    if (fread(magic, sizeof(char), SAVE_MAGIC_SIZE, fp) != SAVE_MAGIC_SIZE ||
        memcmp(magic, SAVE_MAGIC, SAVE_MAGIC_SIZE) != 0) { /* compare file signature bytes*/
        fclose(fp);
        printf("\n>>> ERROR: Save file format is invalid or from an older version. Create a new save with 'save'. <<<\n");
        return -1;
    }

    if (!read_u32(fp, &width_value)
        || !read_u32(fp, &height_value)
        || width_value < 1UL || width_value > (unsigned long)MAX_MAP_WIDTH
        || height_value < 1UL || height_value > (unsigned long)MAX_MAP_HEIGHT) {
        fclose(fp);
        printf("\n>>> ERROR: Save file is corrupted. <<<\n");
        return -1;
    }

    map_width = (int)width_value;
    map_height = (int)height_value;

    if (fread(cave_map, sizeof(char), MAX_MAP_HEIGHT * MAX_MAP_WIDTH, fp) != MAX_MAP_HEIGHT * MAX_MAP_WIDTH
        || !read_entity(fp, &player)
        || !read_entity(fp, &enemy)
        || !read_entity(fp, &weapon)
        || !read_u32(fp, &flag_value)
        || flag_value > 1UL) {
        fclose(fp);
        printf("\n>>> ERROR: Save file is incomplete or corrupted. <<<\n");
        return -1;
    }

    has_weapon = (int)flag_value;

    if (!read_u32(fp, &flag_value)
        || flag_value > 1UL
        || !read_u32(fp, &step_value)
        || step_value > (unsigned long)INT_MAX) {
        fclose(fp);
        printf("\n>>> ERROR: Save file is incomplete or corrupted. <<<\n");
        return -1;
    }

    monster_dead = (int)flag_value;
    loaded_steps = (int)step_value;

    if (!validate_loaded_state(loaded_steps)) {
        fclose(fp);
        printf("\n>>> ERROR: Save file contains invalid game state. <<<\n");
        return -1;
    }

    fclose(fp);
    printf("\n>>> GAME LOADED SUCCESSFULLY! <<<\n");
    return loaded_steps;
}

#define MAX_LEADERBOARD 100

typedef struct {
    char name[4];
    int steps;
} LeaderEntry;

static int load_entries(LeaderEntry *entries, int max) {
    char line[50];
    int count = 0;
    FILE *fp = fopen("leaderboard.csv", "r");
    if (fp == NULL) return 0; /*graceful exit if file doesnt exist*/

    while (fgets(line, sizeof(line), fp) != NULL && count < max) {
        char *name_token = strtok(line, ","); /* split at comma ABC, 123*/
        char *score_token = strtok(NULL, "\n");
        if (name_token != NULL && score_token != NULL) {
            strncpy(entries[count].name, name_token, 3);
            entries[count].name[3] = '\0';
            entries[count].steps = (int)strtol(score_token, NULL, 10);
            count++;
        }
    }
    fclose(fp);
    return count;
}

static void save_entries(LeaderEntry *entries, int count) {
    FILE *fp = fopen("leaderboard.csv", "w");
    int i;
    if (fp == NULL) return;
    for (i = 0; i < count; i++) {
        fprintf(fp, "%s,%d\n", entries[i].name, entries[i].steps);
    }
    fclose(fp);
}

static int compare_entries(const void *a, const void *b) { /*compare steps via qsort*/
    return ((LeaderEntry *)a)->steps - ((LeaderEntry *)b)->steps;
}

void record_score(int steps) {
    char initials[4];
    LeaderEntry entries[MAX_LEADERBOARD];
    int count;

    printf("\n*** YOU ESCAPED! ***\n");
    printf("Enter 3 initials for the leaderboard: ");
    scanf("%3s", initials);

    count = load_entries(entries, MAX_LEADERBOARD - 1);
    strncpy(entries[count].name, initials, 3);
    entries[count].name[3] = '\0';
    entries[count].steps = steps;
    count++;

    qsort(entries, count, sizeof(LeaderEntry), compare_entries);
    save_entries(entries, count);
}

void display_leaderboard(void) {
    LeaderEntry entries[MAX_LEADERBOARD];
    int count;
    int i;

    printf("\n=========================\n");
    printf("   ABYSS LEADERBOARD   \n");
    printf("=========================\n");

    count = load_entries(entries, MAX_LEADERBOARD);
    if (count == 0) {
        printf("  No scores recorded yet!  \n");
        printf("=========================\n\n");
        return;
    }

    for (i = 0; i < count; i++) {
        printf("  %s ........ %d steps\n", entries[i].name, entries[i].steps);
    }

    printf("=========================\n\n");
}
