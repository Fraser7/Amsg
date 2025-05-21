#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "gamestate.h"
void renderScore(SDL_Renderer *renderer, TTF_Font *font, Snake* player1, Snake* player2, double time_remaining);
void drawApple(Apple *apple, SDL_Renderer *renderer);
void drawGrid(SDL_Renderer *renderer);
void drawSnake(Snake *head, SDL_Renderer *renderer, int type);

#endif