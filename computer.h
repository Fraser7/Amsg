#ifndef COMPUTER_H_
#define COMPUTER_H_

#include "snake.h"
#include "apple.h"

bool isValidMoveForBFS(Snake *snake, int x, int y, Snake *otherSnake);
void pixelToCell(int pixelX, int pixelY, int *cellX, int *cellY);
bool bfsFindPathToTarget(Snake *snake, int targetX, int targetY, int *nextMoveX, int *nextMoveY, Snake *otherSnake);
bool safeMove(Snake *snake, int direction, Snake *otherSnake);
int fallbackSafeMove(Snake *snake, Snake *otherSnake);
Snake *copySnake(const Snake *head);
bool isPathSafe(Snake *snake, int nextX, int nextY, Snake *otherSnake);
int getNextDirection(Snake *snake, Apple *apple, Snake *otherSnake);
int computerMove(Snake *snake, Apple *apple, Snake *otherSnake);

#endif // COMPUTER_H_
