#include "apple.h"

void generateApple(Apple** apple, Snake* player1, Snake* player2, int type) {
    // Create a new apple node
    Apple *newApple = (Apple*)malloc(sizeof(Apple));
    if (!newApple) {
        printf("Memory allocation failed!\n");
        return;
    }
    newApple->next = NULL;
    newApple->type = type;

    // Calculate grid boundaries correctly
    int gridOriginX = (WINDOW_WIDTH - CELL_WIDTH) / 2;
    int gridOriginY = (WINDOW_HEIGHT - CELL_HEIGHT) / 2;
    int num_cells_x = CELL_WIDTH / CELL_SIZE;
    int num_cells_y = CELL_HEIGHT / CELL_SIZE;

    bool valid;
    do {
        valid = true;

        // Generate random cell coordinates
        int cell_x = rand() % num_cells_x;
        int cell_y = rand() % num_cells_y;

        // Convert cell coordinates to pixel coordinates
        newApple->x_coord = gridOriginX + (cell_x * CELL_SIZE);
        newApple->y_coord = gridOriginY + (cell_y * CELL_SIZE);

        // Check collision with player1
        Snake* temp = player1;
        while (temp) {
            if (temp->x_coord == newApple->x_coord && temp->y_coord == newApple->y_coord) {
                valid = false;
                break;
            }
            temp = temp->next;
        }

        // Check collision with player2
        if (valid) {
            temp = player2;
            while (temp) {
                if (temp->x_coord == newApple->x_coord && temp->y_coord == newApple->y_coord) {
                    valid = false;
                    break;
                }
                temp = temp->next;
            }
        }

        // Check collision with existing apples
        if (valid) {
            Apple* existing = *apple;
            while (existing) {
                if (existing->x_coord == newApple->x_coord && existing->y_coord == newApple->y_coord) {
                    valid = false;
                    break;
                }
                existing = existing->next;
            }
        }

    } while (!valid);

    // Append the new apple to the linked list
    if (*apple == NULL) {
        *apple = newApple;  // First apple in the list
    } else {
        Apple* temp_a = *apple;
        while (temp_a->next) {
            temp_a = temp_a->next;
        }
        temp_a->next = newApple;  // Append at the end
    }
}


Apple *isEaten(Apple *apple, Snake *player) {
    Apple *temp = apple;
    while (temp) {
        if (abs(temp->x_coord - player->x_coord) < CELL_SIZE &&
            abs(temp->y_coord - player->y_coord) < CELL_SIZE) {
            return temp;  // Return the eaten apple
        }
        temp = temp->next;
    }
    return NULL;
}


void removeApple(Apple **apple, Apple *toRemove) {
    if (*apple == NULL || toRemove == NULL) return;

    Apple *temp = *apple, *prev = NULL;

    // If the apple to remove is the head
    if (temp == toRemove) {
        *apple = temp->next;
        free(temp);
        return;
    }

    // Find the apple in the linked list
    while (temp != NULL && temp != toRemove) {
        prev = temp;
        temp = temp->next;
    }

    // If the apple was found, remove it
    if (temp != NULL) {
        prev->next = temp->next;
        free(temp);
    }
}

