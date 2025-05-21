#include "snake.h"

Snake* createSnakePart(int x_coord, int y_coord, int direction, int velocity, int score, int lives, SDL_Color color) {
    Snake* newBodyPart = (Snake*)malloc(sizeof(Snake));
    if (!newBodyPart) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newBodyPart->x_coord = x_coord;
    newBodyPart->y_coord = y_coord;
    newBodyPart->direction = direction;
    newBodyPart->velocity = velocity;
    newBodyPart->score = score;
    newBodyPart->lives = lives;
    newBodyPart->color = color; 
    newBodyPart->next = NULL;
    return newBodyPart;
}


void insertBodyPart(Snake** head, int x_coord, int y_coord, int direction, int velocity, int score, int lives, SDL_Color color) {
    Snake* newBodyPart = createSnakePart(x_coord, y_coord, direction, velocity, score, lives, color);
    if (*head == NULL) {
        *head = newBodyPart;
        return;
    }

    Snake* temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newBodyPart;
}

Snake* getTail(Snake* head) {
    Snake* temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    return temp;
}


void moveSnake(Snake* head, int direction) {
    if (!head) return;

    // Prevent reversing direction (e.g., UP <-> DOWN or LEFT <-> RIGHT)
    if (fabs(head->direction - direction) == 2) return;

    
    // Store previous positions for body movement
    int prev_x = head->x_coord;
    int prev_y = head->y_coord;
    int prev_direction = head->direction;

    switch (direction) {
        case UP:
            head->y_coord -= CELL_SIZE;
            break;
        case RIGHT:
            head->x_coord += CELL_SIZE;
            break;
        case DOWN:
            head->y_coord += CELL_SIZE;
            break;
        case LEFT:
            head->x_coord -= CELL_SIZE;
            break;
    }
    head->direction = direction;
    



    // Move body segments
    Snake* temp = head->next;
    while (temp) {
        int temp_x = temp->x_coord;
        int temp_y = temp->y_coord;
        int temp_direction = temp->direction;

        temp->x_coord = prev_x;
        temp->y_coord = prev_y;
        temp->direction = prev_direction;

        prev_x = temp_x;
        prev_y = temp_y;
        prev_direction = temp_direction;

        temp->velocity = head->velocity;

        temp = temp->next;
    }
}

int checkCollision(Snake* player1, Snake* player2) {
    Snake* temp1 = player1->next;
    Snake* temp2 = player2->next;

    // Check if player1 collides with the walls
    if (player1->x_coord < (WINDOW_WIDTH - CELL_WIDTH) / 2 || 
        player1->y_coord < (WINDOW_HEIGHT - CELL_HEIGHT) / 2 || 
        player1->x_coord >= (WINDOW_WIDTH + CELL_WIDTH) / 2 || 
        player1->y_coord >= (WINDOW_HEIGHT + CELL_HEIGHT) / 2) 
        return 1;

    // Check if player2 collides with the walls
    if (player2->x_coord < (WINDOW_WIDTH - CELL_WIDTH) / 2 || 
        player2->y_coord < (WINDOW_HEIGHT - CELL_HEIGHT) / 2 || 
        player2->x_coord >= (WINDOW_WIDTH + CELL_WIDTH) / 2 || 
        player2->y_coord >= (WINDOW_HEIGHT + CELL_HEIGHT) / 2) 
        return 2;

    // Check if player1 collides with its own body
    while (temp1 != NULL) {
        if (temp1->x_coord == player1->x_coord && temp1->y_coord == player1->y_coord) 
            return 1;
        temp1 = temp1->next;
    }

    // Check if player2 collides with its own body
    while (temp2 != NULL) {
        if (temp2->x_coord == player2->x_coord && temp2->y_coord == player2->y_coord) 
            return 2;
        temp2 = temp2->next;
    }

    // Check if player1's head collides with player2's body
    temp2 = player2->next;
    while (temp2 != NULL) {
        if (abs(temp2->x_coord - player1->x_coord) < CELL_SIZE && abs(temp2->y_coord - player1->y_coord) < CELL_SIZE) 
            return 1;
        temp2 = temp2->next;
    }

    // Check if player2's head collides with player1's body
    temp1 = player1->next;
    while (temp1 != NULL) {
        if (abs(temp1->x_coord - player2->x_coord) < CELL_SIZE && abs(temp1->y_coord - player2->y_coord) < CELL_SIZE) 
            return 2;
        temp1 = temp1->next;
    }

    // Check if both heads collide (head-on collision)
    if (abs(player1->x_coord - player2->x_coord) < CELL_SIZE && abs(player1->y_coord - player2->y_coord) < CELL_SIZE) 
        return 3;

    return 0;
}



void freeSnake(Snake* head) {
    Snake* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

void removeTail(Snake **head) {
    // If the snake is empty, do nothing
    if (*head == NULL) {
        return;
    }

    // If the snake has only one segment, free it and set head to NULL
    if ((*head)->next == NULL) {
        free(*head);
        *head = NULL;
        return;
    }

    // Traverse to the second last segment
    Snake *current = *head;
    while (current->next->next != NULL) {
        current = current->next;
    }

    // Free the tail and set second last segment's next to NULL
    free(current->next);
    current->next = NULL;
}

void InvincibleCollision(Snake* invincible, Snake** other) {
    
    if (invincible->x_coord < (WINDOW_WIDTH - CELL_WIDTH) / 2 || 
        invincible->y_coord < (WINDOW_HEIGHT - CELL_HEIGHT) / 2 || 
        invincible->x_coord >= (WINDOW_WIDTH + CELL_WIDTH) / 2 || 
        invincible->y_coord >= (WINDOW_HEIGHT + CELL_HEIGHT) / 2){
        invincible->lives--;
    }

    if (*other == NULL)
        return; // Nothing to do if there's no other snake.

    if ((*other)->x_coord < (WINDOW_WIDTH - CELL_WIDTH) / 2 || 
        (*other)->y_coord < (WINDOW_HEIGHT - CELL_HEIGHT) / 2 || 
        (*other)->x_coord >= (WINDOW_WIDTH + CELL_WIDTH) / 2 || 
        (*other)->y_coord >= (WINDOW_HEIGHT + CELL_HEIGHT) / 2){
        (*other)->lives--;
    }

    // --- 2. Check collision with the other snake (if it exists) ---

    if (abs(invincible->x_coord - (*other)->x_coord) < CELL_SIZE && abs(invincible->y_coord - (*other)->y_coord) < CELL_SIZE){
        (*other)->lives--;
    }

    // Otherwise, check collision with any body segment of the other snake.
    Snake* prev = *other;
    Snake* seg = (*other)->next;
    while (seg != NULL) {
        if (invincible->x_coord == seg->x_coord &&
            invincible->y_coord == seg->y_coord) {
            // Collision with a body segment: remove that segment and all segments following it.
            freeSnake(seg);
            prev->next = NULL;  // Trim the snake here.
            break;
        }
        prev = seg;
        seg = seg->next;
    }
}
