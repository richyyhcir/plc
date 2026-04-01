#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAP_WIDTH 60
#define MAP_HEIGHT 20
#define BRUSH_RADIUS 1
#define NUM_DIGGERS 10 /* num of branches for cave generation algo */
#define WALK_STEPS 120

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

/* cave generation algo */
void generate_thick_maze(void) {
    int x, y, step, dir, bx, by, carve_x, carve_y, d;
    
    int digger_x[NUM_DIGGERS];
    int digger_y[NUM_DIGGERS];

    /* fill with solid walls */
    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            cave_map[y][x] = '#';
        }
    }

    /* spawn all diggers in the center of the map */
    for (d = 0; d < NUM_DIGGERS; d++) {
        digger_x[d] = MAP_WIDTH / 2;
        digger_y[d] = MAP_HEIGHT / 2;
    }

    /* set player spawn at the center area */
    player.x = MAP_WIDTH / 2;
    player.y = MAP_HEIGHT / 2;
    player.symbol = '@';

    /* start digging */
    for (step = 0; step < WALK_STEPS; step++) {
        for (d = 0; d < NUM_DIGGERS; d++) {
            /* carve the thick brush around this specific digger */
            for (by = -BRUSH_RADIUS; by <= BRUSH_RADIUS; by++) {
                for (bx = -BRUSH_RADIUS; bx <= BRUSH_RADIUS; bx++) {
                    carve_x = digger_x[d] + bx;
                    carve_y = digger_y[d] + by;
                    /* keep a 1 tile border of solid walls */
                    if (carve_x > 0 && carve_x < MAP_WIDTH - 1 && carve_y > 0 && carve_y < MAP_HEIGHT - 1) {
                        cave_map[carve_y][carve_x] = '.';
                    }
                }
            }

            /* move this digger in a random direction */
            dir = rand() % 4;
            if (dir == 0 && digger_y[d] > BRUSH_RADIUS + 1) digger_y[d]--;
            else if (dir == 1 && digger_y[d] < MAP_HEIGHT - BRUSH_RADIUS - 2) digger_y[d]++;
            else if (dir == 2 && digger_x[d] > BRUSH_RADIUS + 1) digger_x[d]--;
            else if (dir == 3 && digger_x[d] < MAP_WIDTH - BRUSH_RADIUS - 2) digger_x[d]++;
        }
    }
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
                putchar(player.symbol);
            } else {
                putchar(cave_map[y][x]);
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
        generate_thick_maze();
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
        if (cave_map[target_y][target_x] == '.') {
            player.x = target_x;
            player.y = target_y;
            step_count++; /* only count valid steps */
        }
        
        /* if hit wall, do nothing and loop again */
    }

    return 0;
}