#include "display.h"

void renderScore(SDL_Renderer *renderer, TTF_Font *font, Snake *player1, Snake *player2, double time_remaining)
{
    SDL_Color color = {255, 255, 255, 255}; // White color

    char scoreText[150];                                                                                                                                                                // Buffer to store score text
    sprintf(scoreText, "You: Score %d, Lives %d| Time remaining: %.2fs| Opponent: Score %d, Lives %d", player1->score, player1->lives, time_remaining, player2->score, player2->lives); // Convert score to string

    SDL_Surface *textSurface = TTF_RenderText_Solid(font, scoreText, color);
    if (!textSurface)
    {
        printf("Text surface creation failed: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture)
    {
        printf("Text texture creation failed: %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect textRect = {WINDOW_WIDTH / 2 - textSurface->w / 2, 20, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect); // Render text

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void drawApple(Apple *apple, SDL_Renderer *renderer)
{
    SDL_Rect rect;
    rect.w = rect.h = CELL_SIZE;
    Apple *temp = apple;

    while (temp != NULL)
    {
        rect.x = temp->x_coord; // Corrected: Use temp instead of apple
        rect.y = temp->y_coord;

        switch (temp->type)
        {       // Corrected: Use temp->type
        case 0: // normal apple [red]
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            break;
        case 1: // -1 body [green]
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            break;
        case 2: // +2 body [blue]
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            break;
        case 3: // speed increase [yellow]
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            break;
        case 4: // invincibility shield [purple]
            SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255);
            break;
        case 5: // invisibility with less speed and limited vision [black]
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            break;
        case 6: // cannon [orange]
            SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
            break;
        case 7: // walls hacks [grey]
            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
            break;
        case 8: // multiple copies hack [pink]
            SDL_SetRenderDrawColor(renderer, 255, 192, 203, 255);
            break;
        case 9: // time rewind 5s [black and white]
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &rect); // White part
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &rect); // Black border
            break;
        case 10: // splits in half (one AI controlled) [cyan]
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
            break;
        case 11: // head->tail and tail->head [green and blue]
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderDrawRect(renderer, &rect); // Border in blue
            break;
        case 12: // moving apple [red and yellow]
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &rect); // Yellow border
            break;
        case 13: // reverse opponents controls [brown]
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
            break;
        case 14: // snapshot [white]
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            break;
        }

        SDL_RenderFillRect(renderer, &rect); // Render apple
        temp = temp->next;                   // Move to next apple in linked list
    }
}

void drawGrid(SDL_Renderer *renderer)
{
    SDL_Rect grid;
    grid.x = (WINDOW_WIDTH - CELL_WIDTH) / 2;
    grid.y = (WINDOW_HEIGHT - CELL_HEIGHT) / 2;
    grid.w = CELL_WIDTH;
    grid.h = CELL_HEIGHT;

    SDL_SetRenderDrawColor(renderer, 106, 106, 106, 255); // Gray Grid Color
    SDL_RenderDrawRect(renderer, &grid);
}

void drawSnake(Snake *player, SDL_Renderer *renderer, int type)
{
    SDL_Rect rect;
    Snake *curr = player;
    Snake *prev = NULL; // To hold the previous segment in the list

    while (curr != NULL)
    {
        // Draw the filled rectangle (segment)
        rect.x = curr->x_coord;
        rect.y = curr->y_coord;
        rect.w = rect.h = CELL_SIZE;
        SDL_SetRenderDrawColor(renderer, curr->color.r, curr->color.g, curr->color.b, curr->color.a);
        SDL_RenderFillRect(renderer, &rect);

        // Determine neighbor presence using only the adjacent segments in the chain.
        bool neighborTop = false, neighborBottom = false, neighborLeft = false, neighborRight = false;

        // Check the previous segment (if exists)
        if (prev != NULL)
        {
            if (prev->x_coord == rect.x && prev->y_coord == rect.y - CELL_SIZE)
                neighborTop = true;
            if (prev->x_coord == rect.x && prev->y_coord == rect.y + CELL_SIZE)
                neighborBottom = true;
            if (prev->x_coord == rect.x - CELL_SIZE && prev->y_coord == rect.y)
                neighborLeft = true;
            if (prev->x_coord == rect.x + CELL_SIZE && prev->y_coord == rect.y)
                neighborRight = true;
        }
        // Check the next segment (if exists)
        if (curr->next != NULL)
        {
            if (curr->next->x_coord == rect.x && curr->next->y_coord == rect.y - CELL_SIZE)
                neighborTop = true;
            if (curr->next->x_coord == rect.x && curr->next->y_coord == rect.y + CELL_SIZE)
                neighborBottom = true;
            if (curr->next->x_coord == rect.x - CELL_SIZE && curr->next->y_coord == rect.y)
                neighborLeft = true;
            if (curr->next->x_coord == rect.x + CELL_SIZE && curr->next->y_coord == rect.y)
                neighborRight = true;
        }

        // Set outline color based on type
        switch (type)
        {
        case 3:
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            break;
        case 4:
            SDL_SetRenderDrawColor(renderer, 128, 20, 128, 255);
            break;
        }

        // Draw only those edges that are not shared with a neighbor, using 2 pixels thickness.
        if (!neighborTop)
        {
            SDL_RenderDrawLine(renderer, rect.x, rect.y, rect.x + rect.w, rect.y);
            SDL_RenderDrawLine(renderer, rect.x, rect.y + 1, rect.x + rect.w, rect.y + 1);
            SDL_RenderDrawLine(renderer, rect.x, rect.y + 2, rect.x + rect.w, rect.y + 2);
        }
        if (!neighborBottom)
        {
            SDL_RenderDrawLine(renderer, rect.x, rect.y + rect.h, rect.x + rect.w, rect.y + rect.h);
            SDL_RenderDrawLine(renderer, rect.x, rect.y + rect.h - 1, rect.x + rect.w, rect.y + rect.h - 1);
            SDL_RenderDrawLine(renderer, rect.x, rect.y + rect.h - 2, rect.x + rect.w, rect.y + rect.h - 2);
        }
        if (!neighborLeft)
        {
            SDL_RenderDrawLine(renderer, rect.x, rect.y, rect.x, rect.y + rect.h);
            SDL_RenderDrawLine(renderer, rect.x + 1, rect.y, rect.x + 1, rect.y + rect.h);
            SDL_RenderDrawLine(renderer, rect.x + 2, rect.y, rect.x + 2, rect.y + rect.h);
        }
        if (!neighborRight)
        {
            SDL_RenderDrawLine(renderer, rect.x + rect.w, rect.y, rect.x + rect.w, rect.y + rect.h);
            SDL_RenderDrawLine(renderer, rect.x + rect.w - 1, rect.y, rect.x + rect.w - 1, rect.y + rect.h);
            SDL_RenderDrawLine(renderer, rect.x + rect.w - 2, rect.y, rect.x + rect.w - 2, rect.y + rect.h);
        }

        prev = curr;
        curr = curr->next;
    }
}
