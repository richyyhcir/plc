#ifndef GAME_H
#define GAME_H

/* maximum dimensions for array allocation */
#define MAX_CELLS_W 30
#define MAX_CELLS_H 20
#define MAX_MAP_WIDTH  (MAX_CELLS_W * 3 + 1)
#define MAX_MAP_HEIGHT (MAX_CELLS_H * 3 + 1)

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

typedef struct {
    int cells_w;      /* grid width in cells */
    int cells_h;      /* grid height in cells */
    unsigned int seed; /* random seed (0 = use time) */
    int density;       /* wall density 0-100: 100 = perfect maze, 0 = very open */
} Config;

/* derived dimensions from config */
extern int map_width;
extern int map_height;

/* shared globals */
extern char cave_map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
extern Entity player;
extern Entity enemy;
extern Entity weapon;
extern Config config;
extern int has_weapon;
extern int monster_dead;

/* config.c */
int load_config(const char *filename);

/* map.c */
void generate_maze(void);
void render_game(int steps);
int check_line_of_sight(void);

/* enemy.c */
void move_enemy(void);

/* save.c */
void save_game(int steps);
int load_game(void);
void record_score(int steps);
void display_leaderboard(void);

#endif
