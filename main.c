#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
    STATE_GAME_OVER
} GameState;

typedef struct {
    int x;
    int y;
    char symbol;
} Entity;

char cave_map[MAP_HEIGHT][MAP_WIDTH];
Entity player;

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
}

/* rendering */
void render_game(int steps) {
    int x, y;
    
    /* clear console */
    printf("\n\n\n\n\n\n\n\n\n\n"); 
    printf("--- ABYSS WALKER ---\n");
    printf("Steps taken: %d\n", steps);
    printf("Controls: w, a, s, d (then press Enter). Type 'q' to quit.\n\n");

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            /* if the current coordinate matches the player, draw the player instead of the map */
            if (x == player.x && y == player.y) {
                /* putchar(player.symbol); */
                printf("%c ", player.symbol);
            } else {
                /* putchar(cave_map[y][x]); */
                printf("%c ", cave_map[y][x]);
            }
        }
        putchar('\n');
    }
    printf("\nCommand: ");
}

int main(void) {
    GameState current_state = STATE_GENERATING;
    char input;
    int target_x, target_y;
    int step_count = 0;

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
        else if (input == 'q') {
            printf("You have abandoned the abyss.\n");
            current_state = STATE_GAME_OVER;
            continue;
        }

        /* collision detection: only move if the target tile is a floor */
        /* if (cave_map[target_y][target_x] == '.') { */
        if (cave_map[target_y][target_x] == ' ') {
            player.x = target_x;
            player.y = target_y;
            step_count++; /* only count valid steps */

            /* check if player reached the exit opening on the right wall */
            if (player.x == MAP_WIDTH - 1) {
                render_game(step_count);
                printf("You escaped the abyss in %d steps!\n", step_count);
                current_state = STATE_GAME_OVER;
                continue;
            }
        }
        
        /* if hit wall, do nothing and loop again */
    }

    return 0;
}