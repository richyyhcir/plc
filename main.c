#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAP_WIDTH 61  /* must be odd for maze gen */
#define MAP_HEIGHT 21 /* must be odd for maze gen */

/* old cave gen parameters (commented out)
#define BRUSH_RADIUS 1
#define NUM_DIGGERS 7
#define WALK_STEPS 120
*/

typedef enum {
    STATE_GENERATING,
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_LEADERBOARD
} GameState;

typedef struct {
    int x;
    int y;
    char symbol;
} Entity;

char cave_map[MAP_HEIGHT][MAP_WIDTH];
Entity player; /* @ for player */
Entity enemy; /* m for monster*/
Entity weapon; /* b for bow */
int has_weapon = 0; /* 0 = no, 1 = yes */
int monster_dead = 0; /* 0 = alive, 1 = unalived */

/* old cave generation algo (drunkard's walk) - commented out
void generate_thick_maze(void) {
    int x, y, step, dir, bx, by, carve_x, carve_y, d;

    int digger_x[NUM_DIGGERS];
    int digger_y[NUM_DIGGERS];

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            cave_map[y][x] = '#';
        }
    }

    for (d = 0; d < NUM_DIGGERS; d++) {
        digger_x[d] = MAP_WIDTH / 2;
        digger_y[d] = MAP_HEIGHT / 2;
    }

    player.x = MAP_WIDTH / 2;
    player.y = MAP_HEIGHT / 2;
    player.symbol = '@';

    for (step = 0; step < WALK_STEPS; step++) {
        for (d = 0; d < NUM_DIGGERS; d++) {
            for (by = -BRUSH_RADIUS; by <= BRUSH_RADIUS; by++) {
                for (bx = -BRUSH_RADIUS; bx <= BRUSH_RADIUS; bx++) {
                    carve_x = digger_x[d] + bx;
                    carve_y = digger_y[d] + by;
                    if (carve_x > 0 && carve_x < MAP_WIDTH - 1 && carve_y > 0 && carve_y < MAP_HEIGHT - 1) {
                        cave_map[carve_y][carve_x] = '.';
                    }
                }
            }

            dir = rand() % 4;
            if (dir == 0 && digger_y[d] > BRUSH_RADIUS + 1) digger_y[d]--;
            else if (dir == 1 && digger_y[d] < MAP_HEIGHT - BRUSH_RADIUS - 2) digger_y[d]++;
            else if (dir == 2 && digger_x[d] > BRUSH_RADIUS + 1) digger_x[d]--;
            else if (dir == 3 && digger_x[d] < MAP_WIDTH - BRUSH_RADIUS - 2) digger_x[d]++;
        }
    }
}
*/

/* DFS recursive backtracker maze generation for perfect maze, guranteed route */
void generate_maze(void) {
    int x, y, i, tmp;
    int cx, cy, nx, ny;
    int top;

    /* maze cells live at odd coordinates, max cells in each dimension */
    int cells_w = MAP_WIDTH / 2;
    int cells_h = MAP_HEIGHT / 2;

    /* direction offsets: up, down, left, right */
    int dx[4] = { 0, 0, -1, 1 };
    int dy[4] = { -1, 1, 0, 0 };

    /* visited grid for maze cells */
    int visited[MAP_HEIGHT / 2][MAP_WIDTH / 2];

    /* explicit stack for DFS (each entry stores cell x,y) */
    int stack_x[MAP_WIDTH / 2 * MAP_HEIGHT / 2];
    int stack_y[MAP_WIDTH / 2 * MAP_HEIGHT / 2];

    int dirs[4];
    int neighbor_count;

    /* fill entire map with walls */
    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            cave_map[y][x] = '#';
        }
    }

    /* clear visited */
    for (y = 0; y < cells_h; y++) {
        for (x = 0; x < cells_w; x++) {
            visited[y][x] = 0;
        }
    }

    /* start DFS from center cell */
    cx = cells_w / 2;
    cy = cells_h / 2;
    visited[cy][cx] = 1;
    cave_map[cy * 2 + 1][cx * 2 + 1] = ' ';

    top = 0;
    stack_x[top] = cx;
    stack_y[top] = cy;

    while (top >= 0) {
        cx = stack_x[top];
        cy = stack_y[top];

        /* gather unvisited neighbors */
        neighbor_count = 0;
        for (i = 0; i < 4; i++) {
            nx = cx + dx[i];
            ny = cy + dy[i];
            if (nx >= 0 && nx < cells_w && ny >= 0 && ny < cells_h && !visited[ny][nx]) {
                dirs[neighbor_count++] = i;
            }
        }

        if (neighbor_count == 0) {
            /* backtrack */
            top--;
        } else {
            /* shuffle available directions with Fisher-Yates */
            for (i = neighbor_count - 1; i > 0; i--) {
                int j = rand() % (i + 1);
                tmp = dirs[i];
                dirs[i] = dirs[j];
                dirs[j] = tmp;
            }

            /* pick first shuffled direction */
            i = dirs[0];
            nx = cx + dx[i];
            ny = cy + dy[i];

            /* carve the wall between current cell and neighbor */
            cave_map[cy * 2 + 1 + dy[i]][cx * 2 + 1 + dx[i]] = ' ';
            /* carve the neighbor cell */
            cave_map[ny * 2 + 1][nx * 2 + 1] = ' ';

            visited[ny][nx] = 1;
            top++;
            stack_x[top] = nx;
            stack_y[top] = ny;
        }
    }

    /* place player at top-left cell of the maze */
    player.x = 1;
    player.y = 1;
    player.symbol = '@';

    /* open exit on the right wall next to the bottom-right cell */
    cave_map[(cells_h - 1) * 2 + 1][MAP_WIDTH - 1] = ' ';

    /* enemy near exit */
    enemy.x = MAP_WIDTH - 3; 
    enemy.y = (cells_h - 1) * 2 + 1;
    enemy.symbol = 'M';

    /* place weapon somewhere in the maze */
    do {
        weapon.x = (rand() % (MAP_WIDTH / 2)) * 2 + 1;
        weapon.y = (rand() % (MAP_HEIGHT / 2)) * 2 + 1;
    } while (weapon.x == player.x && weapon.y == player.y); /* Don't spawn ON the player */
    weapon.symbol = 'B';
}

/* rendering */
void render_game(int steps) {
    int x, y;
    
    /* clear console */
    printf("\n\n\n\n\n\n\n\n\n\n"); 
    printf("--- MAZE RUNNER ---\n");
    printf("Steps taken: %d\n", steps);
    printf("Controls: w, a, s, d. Type 'f' to Fire. 'v' to Save. 'l' to Load. 'q' to Quit.\n");
    printf("Weapon: %s\n\n", has_weapon ? "Bow" : "None");

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            /* if the current coordinate matches the player, draw the player instead of the map */
            if (x == player.x && y == player.y) {
                /* putchar(player.symbol); */
                printf("%c ", player.symbol);
            }
            /* draw monster */
            else if (!monster_dead && x == enemy.x && y == enemy.y) {
                printf("%c ", enemy.symbol);
            }
            /* draw weapon */
            else if (!has_weapon && x == weapon.x && y == weapon.y) {
                printf("%c ", weapon.symbol);
            }
            else {
                /* putchar(cave_map[y][x]); */
                printf("%c ", cave_map[y][x]);
            }
        }
        putchar('\n');
    }
    printf("\nCommand: ");
}

int check_line_of_sight(void) {
    int start, end, i;

    /* check vertical line of sight */
    if (player.x == enemy.x) {
        start = (player.y < enemy.y) ? player.y : enemy.y;
        end = (player.y > enemy.y) ? player.y : enemy.y;
        for (i = start + 1; i < end; i++) {
            if (cave_map[i][player.x] == '#') return 0; /* wall in the way */
        }
        return 1; /* can shoot */
    } 
    /* check horizontal line of sight */
    else if (player.y == enemy.y) {
        start = (player.x < enemy.x) ? player.x : enemy.x;
        end = (player.x > enemy.x) ? player.x : enemy.x;
        for (i = start + 1; i < end; i++) {
            if (cave_map[player.y][i] == '#') return 0; /* wall in the way */
        }
        return 1; /* can shoot */
    }
    
    return 0; /* not on the same axis */
}

void save_game(int steps) {
    /* open file in write binary mode */
    FILE *fp = fopen("maze.sav", "wb");
    if (fp == NULL) {
        printf("\n>>> ERROR: Could not create save file! <<<\n");
        return;
    }

    /* dump the raw memory of our arrays and structs directly to the file */
    fwrite(cave_map, sizeof(char), MAP_HEIGHT * MAP_WIDTH, fp);
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
    
    /* open file in read binary mode */
    FILE *fp = fopen("maze.sav", "rb");
    if (fp == NULL) {
        printf("\n>>> ERROR: No save file found! <<<\n");
        return -1;
    }

    /* read the bytes back into ram in the exact same order */
    fread(cave_map, sizeof(char), MAP_HEIGHT * MAP_WIDTH, fp);
    fread(&player, sizeof(Entity), 1, fp);
    fread(&enemy, sizeof(Entity), 1, fp);
    fread(&weapon, sizeof(Entity), 1, fp);
    fread(&has_weapon, sizeof(int), 1, fp);
    fread(&monster_dead, sizeof(int), 1, fp);
    fread(&loaded_steps, sizeof(int), 1, fp);

    fclose(fp);
    printf("\n>>> GAME LOADED SUCCESSFULLY! <<<\n");
    return loaded_steps;
}

void record_score(int steps) {
    char initials[4];
    FILE *fp;
    
    printf("\n*** YOU ESCAPED! ***\n");
    printf("Enter 3 initials for the leaderboard: ");
    scanf("%3s", initials);

    /* open in append mode to add to the bottom of the text file */
    fp = fopen("leaderboard.csv", "a");
    if (fp != NULL) {
        fprintf(fp, "%s,%d\n", initials, steps);
        fclose(fp);
    } else {
        printf("Error: Could not save to leaderboard.\n");
    }
}

void display_leaderboard(void) {
    char line[50];
    char *name_token;
    char *score_token;
    FILE *fp;

    printf("\n=========================\n");
    printf("   MAZE LEADERBOARD   \n");
    printf("=========================\n");

    /* open in read mode */
    fp = fopen("leaderboard.csv", "r");
    if (fp == NULL) {
        printf("  No scores recorded yet!  \n");
        printf("=========================\n\n");
        return;
    }

    /* state machine */
    while (fgets(line, sizeof(line), fp) != NULL) {
        name_token = strtok(line, ",");
        score_token = strtok(NULL, "\n");

        if (name_token != NULL && score_token != NULL) {
            /* use strtol to safely convert the string number back into an integer */
            long score = strtol(score_token, NULL, 10);
            printf("  %s ........ %ld steps\n", name_token, score);
        }
    }
    
    fclose(fp);
    printf("=========================\n\n");
}

int main(void) {
    GameState current_state = STATE_GENERATING;
    char input;
    int target_x, target_y;
    int step_count = 0;
    int moved = 0;

    srand((unsigned int)time(NULL));

    if (current_state == STATE_GENERATING) {
        generate_maze();
        current_state = STATE_PLAYING;
    }

    /* main game loop */
    while (current_state == STATE_PLAYING) {
        render_game(step_count);

        /* read input */
        if (scanf(" %c", &input) != 1) {
            break;
        }

        target_x = player.x;
        target_y = player.y;

        if (input == 'w') target_y--;
        else if (input == 's') target_y++;
        else if (input == 'a') target_x--;
        else if (input == 'd') target_x++;
        else if (input == 'f') {
            if (has_weapon) {
                if (!monster_dead && check_line_of_sight()) {
                    monster_dead = 1;
                    /* push a temporary message to the console */
                    printf("\n>>> Pew! You shot the monster! <<<\n");
                } else {
                    printf("\n>>> No clear line of sight to the monster! <<<\n");
                }
            } else {
                printf("\n>>> You don't have a weapon! <<<\n");
            }
            continue; /* dont advance the turn or move the player */
        }
        else if (input == 'q') {
            printf("You have abandoned the maze.\n");
            current_state = STATE_GAME_OVER;
            continue;
        }
        else if (input == 'v') {
            save_game(step_count);
            continue;
        }
        else if (input == 'l') {
            int loaded = load_game();
            if (loaded != -1) {
                step_count = loaded; /* overwrite current steps with loaded steps */
            }
            continue;
        }

        /* collision detection: only move if the target tile is a floor */
        /* if (cave_map[target_y][target_x] == '.') { */
        if (cave_map[target_y][target_x] == ' ') {
            player.x = target_x;
            player.y = target_y;
            step_count++; /* only count valid steps */

            if (!has_weapon && player.x == weapon.x && player.y == weapon.y) {
                has_weapon = 1;
                printf("\n>>> You picked up the bow! Type 'f' to fire at the monster. <<<\n");
            }

            /* check if player reached the exit opening on the right wall */
            if (player.x == MAP_WIDTH - 1) {
                render_game(step_count);

                record_score(step_count);
                current_state = STATE_LEADERBOARD;
                /* printf("You escaped the maze in %d steps!\n", step_count);
                 current_state = STATE_GAME_OVER;*/
                continue;
            }

            /* monster ai */
            if (!monster_dead)
            {
                moved = 0;
                
                /* try to close the horizontal distance first */
                if (player.x > enemy.x && cave_map[enemy.y][enemy.x + 1] == ' ') {
                    enemy.x++;
                    moved = 1;
                } else if (player.x < enemy.x && cave_map[enemy.y][enemy.x - 1] == ' ') {
                    enemy.x--;
                    moved = 1;
                }

                /* if it cant move horizontally, try vertical */
                if (moved == 0) {
                    if (player.y > enemy.y && cave_map[enemy.y + 1][enemy.x] == ' ') {
                        enemy.y++;
                        moved = 1;
                    } else if (player.y < enemy.y && cave_map[enemy.y - 1][enemy.x] == ' ') {
                        enemy.y--;
                        moved = 1;
                    }
                }

                /* if stuck, take a random available step so it doesnt freeze */
                if (moved == 0) {
                    int random_dir = rand() % 4;
                    if (random_dir == 0 && cave_map[enemy.y - 1][enemy.x] == ' ') enemy.y--;
                    else if (random_dir == 1 && cave_map[enemy.y + 1][enemy.x] == ' ') enemy.y++;
                    else if (random_dir == 2 && cave_map[enemy.y][enemy.x - 1] == ' ') enemy.x--;
                    else if (random_dir == 3 && cave_map[enemy.y][enemy.x + 1] == ' ') enemy.x++;
                }

                /* check for the kill condition */
                if (player.x == enemy.x && player.y == enemy.y) {
                    render_game(step_count);
                    printf("\n*** THE MONSTER CAUGHT YOU! GAME OVER. ***\n");
                    current_state = STATE_GAME_OVER;
                    continue;
                }
            }
        }
        
        /* if hit wall, do nothing and loop again */
    }

    if (current_state == STATE_LEADERBOARD) {
        display_leaderboard();
    }

    return 0;
}