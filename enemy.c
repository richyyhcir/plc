#include <stdlib.h>
#include "game.h"

Entity enemy;
static int patrol_x, patrol_y;
static int has_patrol_target = 0;

/* BFS pathfinding, returns 1 if path found, sets next_x/next_y to first step */
static int bfs_next_step(int start_x, int start_y, int goal_x, int goal_y,
                         int *next_x, int *next_y)
{
    int came_from[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
    int queue_x[MAX_MAP_HEIGHT * MAX_MAP_WIDTH];
    int queue_y[MAX_MAP_HEIGHT * MAX_MAP_WIDTH];
    int front = 0, back = 0; /* back and front is the index in the queue arrays, front is 
    the index of the next cell to remove, back is the index to add the next cell*/
    int dx[4] = {0, 0, -1, 1};
    int dy[4] = {-1, 1, 0, 0};
    int x, y, nx, ny, i;

    for (y = 0; y < map_height; y++) /*set all cells to -1 unvisited*/
        for (x = 0; x < map_width; x++)
            came_from[y][x] = -1;

    came_from[start_y][start_x] = 4; /* mark start as visited, 
    0 1 2 3 means reached this tile by moving up, down, left, right from prev tile,
    4 just means start since its none of the directions */
    queue_x[back] = start_x; /*set starting coordinates in the queue array*/
    queue_y[back] = start_y;
    back++; /*since starting cell placed in queue, increment the back index of the queue*/

    while (front < back) {
        x = queue_x[front]; /*dequeue next cell, increment front index*/
        y = queue_y[front];
        front++;

        if (x == goal_x && y == goal_y) {
            /* trace back from goal to start to find the first step */
            while (1) {
                i = came_from[y][x];
                if (x - dx[i] == start_x && y - dy[i] == start_y) { /*take the step that led to current, subtract from 
                    current to backtrack one step, check if is start tile*/
                    *next_x = x; /*if it is the start, then our next step should go that way*/
                    *next_y = y;
                    return 1;
                }
                nx = x - dx[i]; /*backtrack one past step*/
                ny = y - dy[i];
                x = nx; /*note that nx ny are temp variables whereas next_x next_y are return values since
                function only returns one step*/
                y = ny;
            }
        }

        for (i = 0; i < 4; i++) { /*check all directions*/
            nx = x + dx[i];
            ny = y + dy[i];
            if (nx >= 0 && nx < map_width && ny >= 0 && ny < map_height
                && came_from[ny][nx] == -1
                && cave_map[ny][nx] == ' ') { /*if neighbour inside map, unvisited via came_from, is walkable*/
                came_from[ny][nx] = i;
                queue_x[back] = nx; /*new neigbour at the back of the queue*/
                queue_y[back] = ny;
                back++;
            }
        }
    }
    return 0; /* no path found */
}

/* pick a random floor tile as a patrol target */
static void pick_patrol_target(void) {
    int x, y, attempts;

    for (attempts = 0; attempts < 200; attempts++) { /*random x and y within map for 200 tries*/
        x = rand() % map_width;
        y = rand() % map_height;
        if (cave_map[y][x] == ' ') {
            patrol_x = x;
            patrol_y = y;
            has_patrol_target = 1;
            return;
        }
    }
    has_patrol_target = 0;
}

/* move enemy: chase player if within radius, otherwise patrol */
void move_enemy(void) {
    int dist_x, dist_y, distance_sq;
    int next_x, next_y;

    if (monster_dead)
        return;

    dist_x = enemy.x - player.x;
    dist_y = enemy.y - player.y;
    distance_sq = dist_x * dist_x + dist_y * dist_y;

    if (distance_sq <= 5 * 5) {
        /* within radius 5: pathfind toward player */
        if (bfs_next_step(enemy.x, enemy.y, player.x, player.y,
                          &next_x, &next_y)) {
            enemy.x = next_x;
            enemy.y = next_y;
        }
    } else {
        /* patrol: pick a target if we don't have one or reached it */
        if (!has_patrol_target || (enemy.x == patrol_x && enemy.y == patrol_y)) {
            pick_patrol_target();
        }

        if (has_patrol_target) {
            if (bfs_next_step(enemy.x, enemy.y, patrol_x, patrol_y,
                              &next_x, &next_y)) {
                enemy.x = next_x;
                enemy.y = next_y;
            } else {
                pick_patrol_target();
            }
        }
    }
}
