#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "game.h"

Entity player; /*self defined struct*/
Entity weapon;
int has_weapon = 0;
int monster_dead = 0;

int main(void) {
    GameState current_state = STATE_GENERATING;
    char input[16];
    int target_x, target_y;
    int step_count = 0;

    load_config("config.txt");

    /* seed: 0 = random, otherwise use the configured seed */
    if (config.seed == 0)
        srand((unsigned int)time(NULL));
    else
        srand(config.seed);

    if (current_state == STATE_GENERATING) {
        generate_maze();
        current_state = STATE_PLAYING;
    }

    /* main game loop */
    while (current_state == STATE_PLAYING) {
        render_game(step_count);

        if (scanf(" %15s", input) != 1) {
            break;
        }

        target_x = player.x;
        target_y = player.y;

        if (strcmp(input, "w") == 0) target_y--;
        else if (strcmp(input, "s") == 0) target_y++;
        else if (strcmp(input, "a") == 0) target_x--;
        else if (strcmp(input, "d") == 0) target_x++;
        else if (strcmp(input, "f") == 0) {
            if (has_weapon) {
                if (!monster_dead && check_line_of_sight()) {
                    monster_dead = 1;
                    printf("\n>>> Pew! You shot the monster! <<<\n");
                } else if (monster_dead) {
                    printf("\n>>> The monster is already dead! <<<\n");
                } else {
                    printf("\n>>> No clear line of sight to the monster! <<<\n");
                }
            } else {
                printf("\n>>> You don't have a weapon! <<<\n");
            }
            continue;
        }
        else if (strcmp(input, "v") == 0 || strcmp(input, "save") == 0) {
            save_game(step_count);
            continue;
        }
        else if (strcmp(input, "l") == 0 || strcmp(input, "load") == 0) {
            int loaded = load_game();
            if (loaded != -1) {
                step_count = loaded;
            }
            continue;
        }
        else if (strcmp(input, "q") == 0 || strcmp(input, "quit") == 0) {
            printf("You have abandoned the abyss.\n");
            current_state = STATE_GAME_OVER;
            continue;
        }

        /* collision detection: only move if the target tile is a floor */
        if (cave_map[target_y][target_x] == ' ') {
            player.x = target_x;
            player.y = target_y;
            step_count++;

            /* weapon pickup */
            if (!has_weapon && player.x == weapon.x && player.y == weapon.y) {
                has_weapon = 1;
                printf("\n>>> You picked up the bow! Type 'f' to fire at the monster. <<<\n");
            }

            /* check if player reached the exit */
            if (player.x == map_width - 1) {
                render_game(step_count);
                record_score(step_count);
                current_state = STATE_LEADERBOARD;
                continue;
            }
        }

        /* enemy movement */
        move_enemy();

        /* check if monster caught the player */
        if (!monster_dead && enemy.x == player.x && enemy.y == player.y) {
            render_game(step_count);
            printf("\n*** THE MONSTER CAUGHT YOU! GAME OVER. ***\n");
            current_state = STATE_GAME_OVER;
            continue;
        }
    }

    if (current_state == STATE_LEADERBOARD) {
        display_leaderboard();
    }

    return 0;
}
