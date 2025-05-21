#ifndef SNAKE_H_
#define SNAKE_H_

#include "gamestate.h" 

Snake* createSnakePart(int x_coord, int y_coord, int direction, int velocity, int score, int lives, SDL_Color color);
void insertBodyPart(Snake** head, int x_coord, int y_coord, int direction, int velocity, int score, int lives, SDL_Color color);
void moveSnake(Snake* head, int direction);
int checkCollision(Snake* player1, Snake* player2);
Snake* getTail(Snake* head);
void freeSnake(Snake* head);
void removeTail(Snake** head);
void InvincibleCollision(Snake* invincible, Snake** other);


#endif // SNAKE_H