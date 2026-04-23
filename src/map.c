#include <stdio.h>
#include <stdlib.h>
#include "game.h"

char cave_map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
int map_width;
int map_height;

/* remove random internal walls to create alternate paths based on density */
static void apply_density(void) {
    int x, y, removable, i, to_remove;
    int walls_x[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];
    int walls_y[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];

    if (config.density >= 100)
        return; /* perfect maze, nothing to remove */

    /* find all removable walls: wall tiles with floor on opposite sides */
    removable = 0;
    for (y = 1; y < map_height - 1; y++) { /*-1 to avoid going out of bounds*/
        for (x = 1; x < map_width - 1; x++) {
            if (cave_map[y][x] != '#')
                continue; /*skip if not a wall*/
            /* horizontal: floor on left and right */
            if (cave_map[y][x - 1] == ' ' && cave_map[y][x + 1] == ' ') { /*checks if wall is between 2 open spaces*/
                walls_x[removable] = x;
                walls_y[removable] = y;
                removable++;
            }
            /* vertical: floor above and below */
            else if (cave_map[y - 1][x] == ' ' && cave_map[y + 1][x] == ' ') {
                walls_x[removable] = x;
                walls_y[removable] = y;
                removable++;
            }
        }
    }

    /* shuffle the positions of the marked removable walls */
    for (i = removable - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp;
        tmp = walls_x[i]; walls_x[i] = walls_x[j]; walls_x[j] = tmp;
        tmp = walls_y[i]; walls_y[i] = walls_y[j]; walls_y[j] = tmp;
    }

    /* remove (100 - density)% of them */
    to_remove = removable * (100 - config.density) / 100;
    for (i = 0; i < to_remove; i++) {
        cave_map[walls_y[i]][walls_x[i]] = ' ';
    }
}

/* DFS recursive backtracker maze generation — 2x2 cells, 2-wide passages */
void generate_maze(void) {
    int x, y, i, tmp; /*coordinates, loop counter, temp var for swapping values*/
    int cx, cy, nx, ny; /*current coordinates, next coordinates*/
    int top; /*top index of stack*/
    int bx, by; /*backtrack coordinates*/
    int cw = config.cells_w; /*cell width from config*/
    int ch = config.cells_h; /*cell height from config*/

    int dx[4] = { 0, 0, -1, 1 }; /*index changes for each direction*/
    int dy[4] = { -1, 1, 0, 0 };

    int visited[MAX_CELLS_H][MAX_CELLS_W];
    int stack_x[MAX_CELLS_W * MAX_CELLS_H];
    int stack_y[MAX_CELLS_W * MAX_CELLS_H];
    int dirs[4];
    int neighbor_count;

    /* compute actual map dimensions from config */
    map_width = cw * 3 + 1; /* to provide 1 wall 1 cell 1 cell 1 wall */
    map_height = ch * 3 + 1;

    /* fill entire map with walls */
    for (y = 0; y < map_height; y++)
        for (x = 0; x < map_width; x++)
            cave_map[y][x] = '#';

    /* clear visited array with 0*/
    for (y = 0; y < ch; y++)
        for (x = 0; x < cw; x++)
            visited[y][x] = 0;

    /* start DFS from center cell */
    cx = cw / 2;
    cy = ch / 2;
    visited[cy][cx] = 1;

    for (by = 0; by < 2; by++)
        for (bx = 0; bx < 2; bx++)
            cave_map[cy * 3 + 1 + by][cx * 3 + 1 + bx] = ' ';

    top = 0;
    stack_x[top] = cx;
    stack_y[top] = cy;

    while (top >= 0) {
        cx = stack_x[top];
        cy = stack_y[top];

        neighbor_count = 0;
        for (i = 0; i < 4; i++) {
            nx = cx + dx[i];
            ny = cy + dy[i];
            if (nx >= 0 && nx < cw && ny >= 0 && ny < ch && !visited[ny][nx])
                dirs[neighbor_count++] = i;
        }

        if (neighbor_count == 0) {
            top--;
        } else {
            for (i = neighbor_count - 1; i > 0; i--) {
                int j = rand() % (i + 1);
                tmp = dirs[i];
                dirs[i] = dirs[j];
                dirs[j] = tmp;
            }

            i = dirs[0];
            nx = cx + dx[i];
            ny = cy + dy[i];

            if (dx[i] == 1) {
                cave_map[cy * 3 + 1][cx * 3 + 3] = ' ';
                cave_map[cy * 3 + 2][cx * 3 + 3] = ' ';
            } else if (dx[i] == -1) {
                cave_map[cy * 3 + 1][nx * 3 + 3] = ' ';
                cave_map[cy * 3 + 2][nx * 3 + 3] = ' ';
            } else if (dy[i] == 1) {
                cave_map[cy * 3 + 3][cx * 3 + 1] = ' ';
                cave_map[cy * 3 + 3][cx * 3 + 2] = ' ';
            } else {
                cave_map[ny * 3 + 3][cx * 3 + 1] = ' ';
                cave_map[ny * 3 + 3][cx * 3 + 2] = ' ';
            }

            for (by = 0; by < 2; by++)
                for (bx = 0; bx < 2; bx++)
                    cave_map[ny * 3 + 1 + by][nx * 3 + 1 + bx] = ' ';

            visited[ny][nx] = 1;
            top++;
            stack_x[top] = nx;
            stack_y[top] = ny;
        }
    }

    /* apply wall density — remove walls to create shortcuts */
    apply_density();

    /* place player at top-left cell */
    player.x = 1;
    player.y = 1;
    player.symbol = '@';

    /* open exit: 2-wide opening on the right wall next to bottom-right cell */
    cave_map[(ch - 1) * 3 + 1][map_width - 1] = ' ';
    cave_map[(ch - 1) * 3 + 2][map_width - 1] = ' ';

    /* spawn monster near the exit */
    enemy.x = map_width - 3;
    enemy.y = (ch - 1) * 3 + 1;
    enemy.symbol = 'M';

    /* place weapon somewhere in the maze (not on player or enemy) */
    do {
        weapon.x = (rand() % config.cells_w) * 3 + 1;
        weapon.y = (rand() % config.cells_h) * 3 + 1;
    } while ((weapon.x == player.x && weapon.y == player.y) ||
             (weapon.x == enemy.x && weapon.y == enemy.y));
    weapon.symbol = 'B';
}

/* check if player has line of sight to the monster (same row or column, no walls) */
int check_line_of_sight(void) {
    int start, end, i;

    /* check vertical line of sight */
    if (player.x == enemy.x) {
        start = (player.y < enemy.y) ? player.y : enemy.y; /*find smaller y*/
        end = (player.y > enemy.y) ? player.y : enemy.y;
        for (i = start + 1; i < end; i++) {
            if (cave_map[i][player.x] == '#') return 0; /*for each y, find wall #*/
        }
        return 1;
    }
    /* check horizontal line of sight */
    else if (player.y == enemy.y) {
        start = (player.x < enemy.x) ? player.x : enemy.x;
        end = (player.x > enemy.x) ? player.x : enemy.x;
        for (i = start + 1; i < end; i++) {
            if (cave_map[player.y][i] == '#') return 0; /*for each x, find wall #*/
        }
        return 1;
    }

    return 0; /* not on the same axis */
}

/* rendering */
void render_game(int steps) {
    int x, y;

    printf("\n\n\n\n\n\n\n\n\n\n");
    printf("--- ABYSS WALKER ---\n");
    printf("Steps taken: %d\n", steps);
    printf("Controls: w, a, s, d. Type 'f' to Fire. 'v' or 'save'. 'l' or 'load'. 'q' or 'quit'.\n");
    printf("Weapon: %s\n\n", has_weapon ? "Bow" : "None");

    for (y = 0; y < map_height; y++) {
        for (x = 0; x < map_width; x++) {
            if (x == player.x && y == player.y) {
                printf("%c ", player.symbol);
            } else if (!monster_dead && x == enemy.x && y == enemy.y) {
                printf("%c ", enemy.symbol);
            } else if (!has_weapon && x == weapon.x && y == weapon.y) {
                printf("%c ", weapon.symbol);
            } else {
                printf("%c ", cave_map[y][x]); /*default print map tile*/
            }
        }
        putchar('\n');
    }
    printf("\nCommand: ");
}
