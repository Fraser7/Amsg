#ifndef GAMESTATE_H_
#define GAMESTATE_H_

#include "inc/SDL.h"
#include "inc/SDL_ttf.h"
#include "inc/SDL_image.h"
#include "inc/SDL_mixer.h"
#include "inc/SDL_net.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <string.h>

#define WINDOW_HEIGHT 1080
#define WINDOW_WIDTH 1920

#define CELL_HEIGHT 800
#define CELL_WIDTH 1600
#define CELL_SIZE 20

#define GRID_COLS (CELL_WIDTH / CELL_SIZE)
#define GRID_ROWS (CELL_HEIGHT / CELL_SIZE)

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define MAX_SNAKE_LENGTH 200

typedef struct Snake
{
    int x_coord, y_coord;
    int direction;
    int velocity;
    int score;
    int lives;
    SDL_Color color;
    struct Snake *next;
} Snake;

typedef struct Apple
{
    int x_coord;
    int y_coord;
    int type;
    struct Apple *next;
} Apple;

typedef struct
{
    int x, y;
} Point;

typedef struct
{
    Point prev;
    bool visited;
} Cell;

typedef struct
{
    int player_id;
    int score;
    int lives;
    SDL_Color color; // 1 or 2
    int snake_length;
    int snake_x[MAX_SNAKE_LENGTH]; // Store only coordinates instead of a linked list
    int snake_y[MAX_SNAKE_LENGTH];
    int apple_x;
    int apple_y;
} GameUpdatePacket;

typedef struct
{
    Mix_Chunk *effect;
    int duration;
} SoundEffects;

#endif