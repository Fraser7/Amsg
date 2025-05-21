#include "computer.h"

 //This function checks if the move is valid for the BFS
bool isValidMoveForBFS(Snake *snake, int x, int y, Snake *otherSnake) {
    // Find tail
    Snake *tail = getTail(snake);
    bool isTail = (tail->x_coord == x && tail->y_coord == y);

    // Check collision with own snake
    Snake *temp = snake;
    while (temp) {
        if (temp->x_coord == x && temp->y_coord == y) {
            if (!isTail) return false;
        }
        temp = temp->next;
    }

    // Check collision with other snake
    temp = otherSnake;
    while (temp) {
        if (temp->x_coord == x && temp->y_coord == y) {
            return false;
        }
        temp = temp->next;
    }

    return true;
}

void pixelToCell(int pixelX, int pixelY, int *cellX, int *cellY) {
    int gridOriginX = (WINDOW_WIDTH - CELL_WIDTH) / 2;
    int gridOriginY = (WINDOW_HEIGHT - CELL_HEIGHT) / 2;
    *cellX = (pixelX - gridOriginX) / CELL_SIZE;
    *cellY = (pixelY - gridOriginY) / CELL_SIZE;
}

bool bfsFindPathToTarget(Snake *snake, int targetX, int targetY, int *nextMoveX, int *nextMoveY, Snake *otherSnake) {
    Cell grid[GRID_COLS][GRID_ROWS] = {0};

    Point queue[GRID_COLS * GRID_ROWS];
    int front = 0, rear = 0;

    int startCellX, startCellY;
    pixelToCell(snake->x_coord, snake->y_coord, &startCellX, &startCellY);

    int targetCellX, targetCellY;
    pixelToCell(targetX, targetY, &targetCellX, &targetCellY);

    queue[rear++] = (Point){startCellX, startCellY};
    grid[startCellX][startCellY].visited = true;
    grid[startCellX][startCellY].prev = (Point){-1, -1};

    int offsets[4][2] = {{1,0}, {-1,0}, {0,-1}, {0,1}};

    while (front < rear) {
        Point cur = queue[front++];
        if (cur.x == targetCellX && cur.y == targetCellY) {
            Point backtrack = cur;
            while (!(grid[backtrack.x][backtrack.y].prev.x == startCellX &&
                     grid[backtrack.x][backtrack.y].prev.y == startCellY)) {
                backtrack = grid[backtrack.x][backtrack.y].prev;
            }
            int gridOriginX = (WINDOW_WIDTH - CELL_WIDTH) / 2;
            int gridOriginY = (WINDOW_HEIGHT - CELL_HEIGHT) / 2;
            *nextMoveX = backtrack.x * CELL_SIZE + gridOriginX;
            *nextMoveY = backtrack.y * CELL_SIZE + gridOriginY;
            return true;
        }

        for (int i = 0; i < 4; i++) {
            int nx = cur.x + offsets[i][0];
            int ny = cur.y + offsets[i][1];
            if (nx < 0 || nx >= GRID_COLS || ny < 0 || ny >= GRID_ROWS)
                continue;
            if (grid[nx][ny].visited)
                continue;

            int cellPixelX = nx * CELL_SIZE + (WINDOW_WIDTH - CELL_WIDTH) / 2;
            int cellPixelY = ny * CELL_SIZE + (WINDOW_HEIGHT - CELL_HEIGHT) / 2;

            if (!isValidMoveForBFS(snake, cellPixelX, cellPixelY, otherSnake))
                continue;

            grid[nx][ny].visited = true;
            grid[nx][ny].prev = cur;
            queue[rear++] = (Point){nx, ny};
        }
    }
    return false;
}

bool isPathSafe(Snake *player, int nextX, int nextY, Snake *otherSnake) {
    Snake *simulated = copySnake(player);
    simulated->x_coord = nextX;
    simulated->y_coord = nextY;

    // Check collision with other snake
    if (checkCollision(simulated, otherSnake)) {
        freeSnake(simulated);
        return false;
    }

    Snake *tail = getTail(simulated);
    int dummyX = 0, dummyY = 0;
    bool safe = bfsFindPathToTarget(player, tail->x_coord, tail->y_coord, &dummyX, &dummyY, otherSnake);

    freeSnake(simulated);
    return safe;
}

int getNextDirection(Snake *player, Apple *apple, Snake *otherSnake) {
    int nextX = 0, nextY = 0;
    bool inDanger = false;
    
    // Check if the snake is in danger (close to other snake)
    if (otherSnake) {
        int headDist = abs(player->x_coord - otherSnake->x_coord) + abs(player->y_coord - otherSnake->y_coord);
        if (headDist <= 3 * CELL_SIZE) {
            inDanger = true;
        }
    }
    
    // Try to find path to apple
    if (bfsFindPathToTarget(player, apple->x_coord, apple->y_coord, &nextX, &nextY, otherSnake) &&
        isPathSafe(player, nextX, nextY, otherSnake)) {
        
        if (nextX > player->x_coord) return RIGHT;
        else if (nextX < player->x_coord) return LEFT;
        else if (nextY > player->y_coord) return DOWN;
        else return UP;
    }

    // If can't reach apple, try to follow tail
    Snake *tail = getTail(player);
    if (bfsFindPathToTarget(player, tail->x_coord, tail->y_coord, &nextX, &nextY, otherSnake) &&
        isPathSafe(player, nextX, nextY, otherSnake)) {
        
        if (nextX > player->x_coord) return RIGHT;
        else if (nextX < player->x_coord) return LEFT;
        else if (nextY > player->y_coord) return DOWN;
        else return UP;
    }

    // If no safe path found, try to find any safe move
    return fallbackSafeMove(player, otherSnake);
}

int computerMove(Snake *player, Apple *apple, Snake *otherSnake) {
    // Find the best apple to target based on type
    Apple *targetApple = NULL;
    Apple *current = apple;
    
    // Group apples by type for prioritization
    Apple *redApple = NULL;
    int redDist = INT_MAX;
    Apple *yellowApple = NULL;
    int yellowDist = INT_MAX;
    Apple *blueApple = NULL;
    int blueDist = INT_MAX;
    Apple *purpleApple = NULL;
    int purpleDist = INT_MAX;
    Apple *otherApple = NULL;
    int otherDist = INT_MAX;
    
    // First pass: find the closest apple of each type
    while (current) {
        // Skip green apples (type 1, -1 body)
        if (current->type == 1) {
            current = current->next;
            continue;
        }
        
        // Calculate distance to this apple
        int distance = abs(player->x_coord - current->x_coord) + abs(player->y_coord - current->y_coord);
        
        // Check if we can reach this apple
        int nextX, nextY;
        if (bfsFindPathToTarget(player, current->x_coord, current->y_coord, &nextX, &nextY, otherSnake)) {
            // Store the closest apple of each type
            if (current->type == 0 && distance < redDist) { // Red apple
                redApple = current;
                redDist = distance;
            } else if (current->type == 3 && distance < yellowDist) { // Yellow apple (speed)
                yellowApple = current;
                yellowDist = distance;
            } else if (current->type == 2 && distance < blueDist) { // Blue apple (+2 body)
                blueApple = current;
                blueDist = distance;
            } else if (current->type == 4 && distance < purpleDist) { // Purple apple
                purpleApple = current;
                purpleDist = distance;
            } else if (current->type != 1 && distance < otherDist) { // Other non-green apples
                otherApple = current;
                otherDist = distance;
            }
        }
        current = current->next;
    }
    
    // Prioritize apples based on both type and distance
    // First, check for very close apples of any priority type
    int proximityThreshold = 5 * CELL_SIZE; // Threshold for considering an apple "nearby"
    
    if (purpleDist < proximityThreshold) {
        targetApple = purpleApple;
    } else if (redDist < proximityThreshold) {
        targetApple = redApple;
    } else if (yellowDist < proximityThreshold) {
        targetApple = yellowApple;
    } else if (blueDist < proximityThreshold) {
        targetApple = blueApple;
    } 
    // If no close priority apples, pick the absolute closest of the priority types
    else if (purpleApple && (purpleDist <= redDist || !redApple) && 
             (purpleDist <= yellowDist || !yellowApple) && (purpleDist <= blueDist || !blueApple)) {
        targetApple = purpleApple;
    } else if (redApple && (redDist <= yellowDist || !yellowApple) && (redDist <= blueDist || !blueApple)) {
        targetApple = redApple;
    } else if (yellowApple && (yellowDist <= blueDist || !blueApple)) {
        targetApple = yellowApple;
    } else if (blueApple) {
        targetApple = blueApple;
    } else if (otherApple) {
        targetApple = otherApple;
    }
    
    // If no suitable apple was found, use the first available non-green apple
    if (!targetApple) {
        current = apple;
        int closestNonGreenDist = INT_MAX;
        while (current) {
            if (current->type != 1) { // Skip green apples
                int distance = abs(player->x_coord - current->x_coord) + abs(player->y_coord - current->y_coord);
                if (distance < closestNonGreenDist) {
                    int nextX, nextY;
                    if (bfsFindPathToTarget(player, current->x_coord, current->y_coord, &nextX, &nextY, otherSnake)) {
                        targetApple = current;
                        closestNonGreenDist = distance;
                    }
                }
            }
            current = current->next;
        }
    }
    
    // If still no target, use the first apple (even if green), but only as a last resort
    if (!targetApple && apple) {
        // First try to find any non-green apple
        current = apple;
        while (current) {
            if (current->type != 1) {
                targetApple = current;
                break;
            }
            current = current->next;
        }
        
        // If there are only green apples, pick one only if we can't find a safe path to follow tail
        if (!targetApple) {
            // Try to follow tail instead of going for green apple
            Snake *tail = getTail(player);
            int nextX, nextY;
            if (bfsFindPathToTarget(player, tail->x_coord, tail->y_coord, &nextX, &nextY, otherSnake)) {
                // Move towards tail
                if (nextX > player->x_coord) {
                    moveSnake(player, RIGHT);
                } else if (nextX < player->x_coord) {
                    moveSnake(player, LEFT);
                } else if (nextY > player->y_coord) {
                    moveSnake(player, DOWN);
                } else {
                    moveSnake(player, UP);
                }
                return 0;
            } else {
                // If we can't follow tail, then as a last resort pick the first available apple
                targetApple = apple;
            }
        }
    }
    
    // Get and apply the next direction
    if (targetApple) {
        int nextDirection = getNextDirection(player, targetApple, otherSnake);
        moveSnake(player, nextDirection);
    } else {
        // No apple available, just move safely
        int nextDirection = fallbackSafeMove(player, otherSnake);
        moveSnake(player, nextDirection);
    }
    
    return 0; // No special power activation
}

bool safeMove(Snake *snake, int direction, Snake *otherSnake) {
    // Calculate the next position based on direction
    int nextX = snake->x_coord;
    int nextY = snake->y_coord;

    switch (direction) {
        case UP:
            nextY -= CELL_SIZE;
            break;
        case DOWN:
            nextY += CELL_SIZE;
            break;
        case LEFT:
            nextX -= CELL_SIZE;
            break;
        case RIGHT:
            nextX += CELL_SIZE;
            break;
    }

    // Check if the move is within grid bounds
    int gridOriginX = (WINDOW_WIDTH - CELL_WIDTH) / 2;
    int gridOriginY = (WINDOW_HEIGHT - CELL_HEIGHT) / 2;
    if (nextX < gridOriginX || 
        nextY < gridOriginY || 
        nextX > gridOriginX + CELL_WIDTH - CELL_SIZE || 
        nextY > gridOriginY + CELL_HEIGHT - CELL_SIZE) {
        return false;
    }

    // Check if the move would cause a collision with the snake's own body
    Snake *current = snake->next;  // Skip the head
    while (current != NULL) {
        if (current->x_coord == nextX && current->y_coord == nextY) {
            return false;
        }
        current = current->next;
    }

    // Check if the move would cause a collision with the other snake
    current = otherSnake;
    while (current != NULL) {
        if (current->x_coord == nextX && current->y_coord == nextY) {
            return false;
        }
        current = current->next;
    }

    return true;
}

int fallbackSafeMove(Snake *snake, Snake *otherSnake) {
    // Try each direction in order: UP, RIGHT, DOWN, LEFT
    int directions[] = {UP, RIGHT, DOWN, LEFT};
    
    // First try the current direction
    if (safeMove(snake, snake->direction, otherSnake)) {
        return snake->direction;
    }

    // Then try other directions
    for (int i = 0; i < 4; i++) {
        if (directions[i] != snake->direction && safeMove(snake, directions[i], otherSnake)) {
            return directions[i];
        }
    }

    // If no safe move is found, return the current direction
    // This is a last resort and might lead to a collision
    return snake->direction;
}

Snake *copySnake(const Snake *head) {
    if (head == NULL) return NULL;
    
    // Create new head with same properties
    Snake *newHead = malloc(sizeof(Snake));
    if (newHead == NULL) return NULL;
    
    // Copy properties from original head
    newHead->x_coord = head->x_coord;
    newHead->y_coord = head->y_coord;
    newHead->direction = head->direction;
    newHead->velocity = head->velocity;
    newHead->score = head->score;
    newHead->lives = head->lives;
    newHead->color = head->color;
    newHead->next = NULL;
    
    // Copy remaining body parts
    Snake *current = head->next;
    Snake *newCurrent = newHead;
    
    while (current != NULL) {
        Snake *newPart = malloc(sizeof(Snake));
        if (newPart == NULL) {
            // Clean up if memory allocation fails
            freeSnake(newHead);
            return NULL;
        }
        
        // Copy properties
        newPart->x_coord = current->x_coord;
        newPart->y_coord = current->y_coord;
        newPart->direction = current->direction;
        newPart->velocity = current->velocity;
        newPart->score = current->score;
        newPart->lives = current->lives;
        newPart->color = current->color;
        newPart->next = NULL;
        
        // Link the new part
        newCurrent->next = newPart;
        newCurrent = newPart;
        current = current->next;
    }
    
    return newHead;
}

