#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

#define SAVE_MAGIC "ABYSS1"
#define SAVE_MAGIC_SIZE 6

void save_game(int steps) {
    FILE *fp = fopen("maze.sav", "wb");
    if (fp == NULL) {
        printf("\n>>> ERROR: Could not create save file! <<<\n");
        return;
    }

    /* write a small file signature so load can reject invalid or old saves safely */
    fwrite(SAVE_MAGIC, sizeof(char), SAVE_MAGIC_SIZE, fp);
    fwrite(&map_width, sizeof(int), 1, fp);
    fwrite(&map_height, sizeof(int), 1, fp);
    fwrite(cave_map, sizeof(char), MAX_MAP_HEIGHT * MAX_MAP_WIDTH, fp);
    fwrite(&player, sizeof(Entity), 1, fp);
    fwrite(&enemy, sizeof(Entity), 1, fp);
    fwrite(&weapon, sizeof(Entity), 1, fp);
    fwrite(&has_weapon, sizeof(int), 1, fp);
    fwrite(&monster_dead, sizeof(int), 1, fp);
    fwrite(&steps, sizeof(int), 1, fp);

    fclose(fp);
    printf("\n>>> GAME SAVED SUCCESSFULLY TO maze.sav <<<\n");
}

/* returns the loaded step count, or -1 if the file doesnt exist */
int load_game(void) {
    int loaded_steps;
    char magic[SAVE_MAGIC_SIZE];

    FILE *fp = fopen("maze.sav", "rb");
    if (fp == NULL) {
        printf("\n>>> ERROR: No save file found! <<<\n");
        return -1;
    }

    if (fread(magic, sizeof(char), SAVE_MAGIC_SIZE, fp) != SAVE_MAGIC_SIZE ||
        memcmp(magic, SAVE_MAGIC, SAVE_MAGIC_SIZE) != 0) {
        fclose(fp);
        printf("\n>>> ERROR: Save file format is invalid or from an older version. Create a new save with 'save'. <<<\n");
        return -1;
    }

    if (fread(&map_width, sizeof(int), 1, fp) != 1 ||
        fread(&map_height, sizeof(int), 1, fp) != 1 ||
        map_width < 1 || map_width > MAX_MAP_WIDTH ||
        map_height < 1 || map_height > MAX_MAP_HEIGHT) {
        fclose(fp);
        printf("\n>>> ERROR: Save file is corrupted. <<<\n");
        return -1;
    }

    if (fread(cave_map, sizeof(char), MAX_MAP_HEIGHT * MAX_MAP_WIDTH, fp) != MAX_MAP_HEIGHT * MAX_MAP_WIDTH ||
        fread(&player, sizeof(Entity), 1, fp) != 1 ||
        fread(&enemy, sizeof(Entity), 1, fp) != 1 ||
        fread(&weapon, sizeof(Entity), 1, fp) != 1 ||
        fread(&has_weapon, sizeof(int), 1, fp) != 1 ||
        fread(&monster_dead, sizeof(int), 1, fp) != 1 ||
        fread(&loaded_steps, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        printf("\n>>> ERROR: Save file is incomplete or corrupted. <<<\n");
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
    if (fp == NULL) return 0;

    while (fgets(line, sizeof(line), fp) != NULL && count < max) {
        char *name_token = strtok(line, ",");
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

static int compare_entries(const void *a, const void *b) {
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
