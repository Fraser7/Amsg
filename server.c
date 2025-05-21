#include "snake.h"
#include "apple.h"
#include "display.h"
#include "computer.h"
#undef main

#define MAX_PLAYERS 2
#define BUFFER_SIZE 10000
#define MAX_CLIENTS 2

int getSnakeLength(Snake *snake)
{
    int length = 0;
    while (snake != NULL)
    {
        length++;
        snake = snake->next;
    }
    return length;
}

int getApples(Apple *apple)
{
    int count = 0;
    while (apple != NULL)
    {
        count++;
        apple = apple->next;
    }
    return count;
}

void fillSnakeData(Snake *snake, int x[], int y[])
{
    int index = 0;
    while (snake != NULL)
    {
        x[index] = snake->x_coord;
        y[index] = snake->y_coord;
        index++;
        snake = snake->next;
    }
}

void fillAppleData(Apple *apple, int x[], int y[], int type[])
{
    int index = 0;
    while (apple != NULL)
    {
        x[index] = apple->x_coord;
        y[index] = apple->y_coord;
        type[index] = apple->type;
        index++;
        apple = apple->next;
    }
}

void handleClientInput(UDPpacket *packet, int client_index, Snake *player1, Snake *player2, int *direction1, int *direction2, int totalModifierPlayer1[], int totalModifierPlayer2[], int activeModifierPlayer1[], int activeModifierPlayer2[], uint32_t startTimeModifierPlayer1[], uint32_t startTimeModifierPlayer2[], int *modifierTypePlayer1, int *modifierTypePlayer2)
{
    char input = ((char *)packet->data)[0]; // Extract 'W', 'A', 'S', 'D'
    printf("Received input: %c\n", input);
    // Update the snake's direction based on the input
    if (client_index == 0)
    {
        if (input == 'W' && player1->direction != DOWN)
            *direction1 = UP;
        if (input == 'A' && player1->direction != RIGHT)
            *direction1 = LEFT;
        if (input == 'S' && player1->direction != UP)
            *direction1 = DOWN;
        if (input == 'D' && player1->direction != LEFT)
            *direction1 = RIGHT;
        if (input == 'I' && totalModifierPlayer1[4] > 0)
        {
            totalModifierPlayer1[4]--;
            activeModifierPlayer1[4] = 1;
            startTimeModifierPlayer1[4] = SDL_GetTicks();
            *modifierTypePlayer1 = 4;
        }
    }
    else
    {
        if (input == 'W' && player2->direction != DOWN)
            *direction2 = UP;
        if (input == 'A' && player2->direction != RIGHT)
            *direction2 = LEFT;
        if (input == 'S' && player2->direction != UP)
            *direction2 = DOWN;
        if (input == 'D' && player2->direction != LEFT)
            *direction2 = RIGHT;
        if (input == 'I' && totalModifierPlayer2[4] > 0)
        {
            totalModifierPlayer2[4]--;
            activeModifierPlayer2[4] = 1;
            startTimeModifierPlayer2[4] = SDL_GetTicks();
            *modifierTypePlayer2 = 4;
        }
    }
}

void sendGameUpdate(UDPsocket server, UDPpacket *packet, int client_channels[], Snake *player1, Snake *player2, Apple *apple, int gameOver, uint32_t timeStart, int modifierTypePlayer1, int modifierTypePlayer2, int soundEffect)
{
    int length1 = getSnakeLength(player1);
    int length2 = getSnakeLength(player2);

    int score1 = player1->score;
    int score2 = player2->score;
    int lives1 = player1->lives;
    int lives2 = player2->lives;

    int apple_length = getApples(apple);
    int *apple_x = (int *)malloc(apple_length * sizeof(int));
    int *apple_y = (int *)malloc(apple_length * sizeof(int));
    int *apple_type = (int *)malloc(apple_length * sizeof(int));
    fillAppleData(apple, apple_x, apple_y, apple_type);

    int *x1 = (int *)malloc(length1 * sizeof(int));
    int *y1 = (int *)malloc(length1 * sizeof(int));
    int *x2 = (int *)malloc(length2 * sizeof(int));
    int *y2 = (int *)malloc(length2 * sizeof(int));
    fillSnakeData(player1, x1, y1);
    fillSnakeData(player2, x2, y2);

    // Calculate time remaining
    uint32_t currentTime = SDL_GetTicks();
    double timeRemaining = 180.0 - ((double)(currentTime - timeStart) / 1000.0);
    if (timeRemaining < 0)
        timeRemaining = 0;

    // Calculate total packet size including player ID and time remaining
    int totalSize = 12 * sizeof(int) + (length1 + length2) * 2 * sizeof(int) + sizeof(double) + 3 * apple_length * sizeof(int);

    // Constructing the data packet
    int offset = 0;
    memcpy(packet->data + offset, &gameOver, sizeof(int));
    offset += sizeof(int);
    memcpy(packet->data + offset, &soundEffect, sizeof(int));
    offset += sizeof(int);
    memcpy(packet->data + offset, &apple_length, sizeof(int));
    offset += sizeof(int);
    memcpy(packet->data + offset, apple_x, apple_length * sizeof(int));
    offset += apple_length * sizeof(int);
    memcpy(packet->data + offset, apple_y, apple_length * sizeof(int));
    offset += apple_length * sizeof(int);
    memcpy(packet->data + offset, apple_type, apple_length * sizeof(int));
    offset += apple_length * sizeof(int);
    memcpy(packet->data + offset, &length1, sizeof(int));
    offset += sizeof(int);
    memcpy(packet->data + offset, x1, length1 * sizeof(int));
    offset += length1 * sizeof(int);
    memcpy(packet->data + offset, y1, length1 * sizeof(int));
    offset += length1 * sizeof(int);
    memcpy(packet->data + offset, &length2, sizeof(int));
    offset += sizeof(int);
    memcpy(packet->data + offset, x2, length2 * sizeof(int));
    offset += length2 * sizeof(int);
    memcpy(packet->data + offset, y2, length2 * sizeof(int));
    offset += length2 * sizeof(int);
    memcpy(packet->data + offset, &score1, sizeof(int));
    offset += sizeof(int);
    memcpy(packet->data + offset, &score2, sizeof(int));
    offset += sizeof(int);
    memcpy(packet->data + offset, &lives1, sizeof(int));
    offset += sizeof(int);
    memcpy(packet->data + offset, &lives2, sizeof(int));
    offset += sizeof(int);
    memcpy(packet->data + offset, &modifierTypePlayer1, sizeof(int));
    offset += sizeof(int);
    memcpy(packet->data + offset, &modifierTypePlayer2, sizeof(int));
    offset += sizeof(int);
    memcpy(packet->data + offset, &timeRemaining, sizeof(double));
    offset += sizeof(double);

    // Send to both clients
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_channels[i] != -1)
        {
            // Add player ID for this specific client
            memcpy(packet->data + offset, &i, sizeof(int));
            packet->len = totalSize;

            if (SDLNet_UDP_Send(server, client_channels[i], packet) == 0)
            {
                printf("Failed to send packet to client %d: %s\n", i, SDLNet_GetError());
            }
        }
    }

    free(x1);
    free(y1);
    free(x2);
    free(y2);
    free(apple_x);
    free(apple_y);
    free(apple_type);
}

void sendGameOver(UDPsocket server, UDPpacket *packet, int client_channels[], int score1, int score2)
{
    int gameOver = 1;
    memcpy(packet->data, &gameOver, sizeof(int));
    packet->len = sizeof(int);

    // Send to both clients
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_channels[i] != -1)
        {
            if (SDLNet_UDP_Send(server, client_channels[i], packet) == 0)
            {
                printf("Failed to send game over packet to client %d: %s\n", i, SDLNet_GetError());
            }
        }
    }
}

int main(int argc, char *argv[])
{
    system("ipconfig");
    printf("\n\nIPv4 Address (Wifi LAN) is your server IP\n");
    srand((unsigned)time(NULL));

    SDL_Color green = {0, 255, 0, 255};
    SDL_Color blue = {0, 0, 255, 255};
    SDL_Color greenHead = {0, 150, 0, 255}; // Much darker green for head
    SDL_Color blueHead = {0, 0, 150, 255};  // Much darker blue for head

    Snake *player1 = createSnakePart(400, 420, 1, 100, 0, 3, greenHead);
    insertBodyPart(&player1, 380, 420, 1, 100, 0, 3, green);
    insertBodyPart(&player1, 360, 420, 1, 100, 0, 3, green);
    // player1->velocity = rand()%30;

    Snake *player2 = createSnakePart(400, 820, 1, 100, 0, 3, blueHead);
    insertBodyPart(&player2, 380, 820, 1, 100, 0, 3, blue);
    insertBodyPart(&player2, 360, 820, 1, 100, 0, 3, blue);
    // player2->velocity = rand()%30;

    Apple *apple = NULL;
    // generateApple(&apple, player1, player2, 0);
    // apple = generateApple(apple, player1, player2, 1);
    int direction1 = RIGHT; // Default to RIGHT
    int direction2 = RIGHT; // Default to RIGHT

    uint32_t lastMoveTime1 = SDL_GetTicks();
    uint32_t lastMoveTime2 = SDL_GetTicks();
    uint32_t lastSpeed = SDL_GetTicks();
    uint32_t timeStart = SDL_GetTicks();
    uint32_t AppleChange = SDL_GetTicks();

    if (SDLNet_Init() < 0)
    {
        printf("SDLNet Initialization failed: %s\n", SDLNet_GetError());
        return 1;
    }

    UDPsocket server = SDLNet_UDP_Open(8888);
    if (!server)
    {
        printf("Failed to open UDP socket: %s\n", SDLNet_GetError());
        return 1;
    }

    UDPpacket *packet = SDLNet_AllocPacket(BUFFER_SIZE);
    if (!packet)
    {
        printf("Failed to allocate packet memory\n");
        return 1;
    }

    IPaddress client_addresses[MAX_CLIENTS] = {0}; // Store client addresses
    int client_channels[MAX_CLIENTS] = {-1, -1};
    int soundEffect = -1;
    int applesEatenFor_2 = 0;
    int applesEatenFor_3 = 0;
    int applesEatenFor_4 = 0;
    int flag = 0;
    int gameOver = 0;
    int Winner = -1;
    int modifierTypePlayer1 = 0;
    int modifierTypePlayer2 = 0;
    // No. of modifiers
    int totalModifierPlayer1[15];
    int totalModifierPlayer2[15];
    // Active or not
    int activeModifierPlayer1[15];
    int activeModifierPlayer2[15];
    uint32_t startTimeModifierPlayer1[15];
    uint32_t startTimeModifierPlayer2[15];
    uint32_t rightNowPlayer1[15];
    uint32_t rightNowPlayer2[15];
    for (int i = 0; i < 15; i++)
    {
        totalModifierPlayer1[i] = 0;
        totalModifierPlayer2[i] = 0;
        activeModifierPlayer1[i] = 0;
        activeModifierPlayer2[i] = 0;
        startTimeModifierPlayer1[i] = 0;
        startTimeModifierPlayer2[i] = 0;
        rightNowPlayer1[i] = 0;
        rightNowPlayer2[i] = 0;
    }

    while (!gameOver)
    {
        // Handle new connections
        if (!flag)
        {
            if (SDLNet_UDP_Recv(server, packet))
            {
                int client_index = -1;
                char input = ((char *)packet->data)[0];

                // Check if this is a connection packet
                if (input == 'C')
                {
                    // Check if the sender is already registered
                    for (int i = 0; i < MAX_CLIENTS; i++)
                    {
                        if (client_channels[i] != -1 &&
                            client_addresses[i].host == packet->address.host &&
                            client_addresses[i].port == packet->address.port)
                        {
                            client_index = i;
                            break;
                        }
                    }

                    // If the client is new, register it
                    if (client_index == -1)
                    {
                        for (int i = 0; i < MAX_CLIENTS; i++)
                        {
                            if (client_channels[i] == -1)
                            {
                                client_addresses[i] = packet->address;
                                client_channels[i] = SDLNet_UDP_Bind(server, i, &packet->address);
                                client_index = i;
                                printf("New client connected on channel %d\n", i);
                                break;
                            }
                        }
                    }

                    if (client_index == -1)
                    {
                        printf("Server is full! Ignoring packet.\n");
                        continue;
                    }

                    if (client_index == 1)
                    {
                        printf("Connected both clients!\n");
                        flag = 1;
                        timeStart = SDL_GetTicks(); // Reset timer when both clients are connected

                        // Reset game state for both players
                        freeSnake(player1);
                        freeSnake(player2);
                        free(apple);

                        // Recreate snakes with initial positions
                        player1 = createSnakePart(400, 420, RIGHT, 100, 0, 3, greenHead);
                        insertBodyPart(&player1, 380, 420, RIGHT, 100, 0, 3, green);
                        insertBodyPart(&player1, 360, 420, RIGHT, 100, 0, 3, green);

                        player2 = createSnakePart(400, 820, RIGHT, 100, 0, 3, blueHead);
                        insertBodyPart(&player2, 380, 820, RIGHT, 100, 0, 3, blue);
                        insertBodyPart(&player2, 360, 820, RIGHT, 100, 0, 3, blue);

                        generateApple(&apple, player1, player2, 0);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        generateApple(&apple, player1, player2, 1);
                        // generateApple(&apple, player1, player2, 2);
                        // generateApple(&apple, player1, player2, 3);
                        // generateApple(&apple, player1, player2, 4);

                        // Reset directions
                        direction1 = RIGHT;
                        direction2 = RIGHT;

                        // Reset movement timers
                        lastMoveTime1 = SDL_GetTicks();
                        lastMoveTime2 = SDL_GetTicks();

                        // Send initial game state to both clients
                        sendGameUpdate(server, packet, client_channels, player1, player2, apple, 0, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                    }
                }
            }
        }

        // Game loop
        if (flag)
        {
            // Check for client input
            if (SDLNet_UDP_Recv(server, packet))
            {
                int client_index = -1;

                // Check if the sender is already registered
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (client_channels[i] != -1 &&
                        client_addresses[i].host == packet->address.host &&
                        client_addresses[i].port == packet->address.port)
                    {
                        client_index = i;
                        break;
                    }
                }

                if (client_index != -1)
                {
                    handleClientInput(packet, client_index, player1, player2, &direction1, &direction2, totalModifierPlayer1, totalModifierPlayer2, activeModifierPlayer1, activeModifierPlayer2, startTimeModifierPlayer1, startTimeModifierPlayer2, &modifierTypePlayer1, &modifierTypePlayer2);
                    printf("Received input from client %d\n", client_index);
                }
            }

            switch (checkCollision(player1, player2))
            {
            case 0:
                break;

            case 1:
                printf("Player 1 collided! Score1:%d Score2:%d\n", player1->score, player2->score);
                if (activeModifierPlayer1[4])
                {
                    int temp1s = player1->score;
                    int temp1l = player1->lives;
                    int temp2s = player2->score;
                    int temp2l = player2->lives;

                    InvincibleCollision(player1, &player2);

                    // Check if either player is out of lives
                    if (player1->lives <= 0 || player2->lives <= 0)
                    {
                        if (player1->lives <= 0 && player2->lives <= 0)
                        {
                            // Both players out of lives - determine winner by score
                            printf("Both players out of lives! ");
                            if (temp1s > temp2s)
                            {
                                printf("Player 1 wins by score!\n");
                            }
                            else if (temp1s < temp2s)
                            {
                                printf("Player 2 wins by score!\n");
                            }
                            else
                            {
                                printf("Game ended in a draw!\n");
                            }
                        }
                        else if (player1->lives <= 0)
                        {
                            printf("Player 1 out of lives! Player 2 wins!\n");
                        }
                        else
                        {
                            printf("Player 2 out of lives! Player 1 wins!\n");
                        }

                        // Send final game update
                        sendGameUpdate(server, packet, client_channels, player1, player2, apple, 1, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                        gameOver = 1;
                    }

                    if (player1->lives < temp1l)
                    {
                        // Player still has lives, reset snake position without ending game
                        freeSnake(player1);
                        player1 = createSnakePart(400, 420, RIGHT, 100, temp1l, temp1s, greenHead);
                        insertBodyPart(&player1, 380, 420, RIGHT, 100, temp1l, temp1s, green);
                        insertBodyPart(&player1, 360, 420, RIGHT, 100, temp1l, temp1s, green);
                        direction1 = RIGHT;
                        activeModifierPlayer1[4] = 0;
                        activeModifierPlayer1[3] = 0;
                        modifierTypePlayer1 = 0;
                        sendGameUpdate(server, packet, client_channels, player1, player2, apple, 0, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                    }
                    if (player2->lives < temp2l)
                    {
                        // Player still has lives, reset snake position without ending game
                        freeSnake(player2);
                        player2 = createSnakePart(400, 820, RIGHT, 100, temp2l, temp2s, blueHead);
                        insertBodyPart(&player2, 380, 820, RIGHT, 100, temp2l, temp2s, blue);
                        insertBodyPart(&player2, 360, 820, RIGHT, 100, temp2l, temp2s, blue);
                        direction2 = RIGHT;
                        activeModifierPlayer2[4] = 0;
                        activeModifierPlayer2[3] = 0;
                        modifierTypePlayer2 = 0;
                        sendGameUpdate(server, packet, client_channels, player1, player2, apple, 0, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                    }
                }
                else
                {
                    int temp = player1->score;
                    int temp2 = --player1->lives;

                    // Check if player has lives left
                    if (player1->lives <= 0)
                    {
                        printf("Player 1 out of lives! Player 2 wins!\n");
                        // Send final game update with zero lives for player 1
                        sendGameUpdate(server, packet, client_channels, player1, player2, apple, 1, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                        gameOver = 1;
                        break;
                    }

                    // Player still has lives, reset snake position without ending game
                    freeSnake(player1);
                    player1 = createSnakePart(400, 420, RIGHT, 100, temp, temp2, greenHead);
                    insertBodyPart(&player1, 380, 420, RIGHT, 100, temp, temp2, green);
                    insertBodyPart(&player1, 360, 420, RIGHT, 100, temp, temp2, green);
                    direction1 = RIGHT;
                    activeModifierPlayer1[4] = 0;
                    activeModifierPlayer1[3] = 0;
                    modifierTypePlayer1 = 0;
                    // Send game update without game over flag
                    // sendGameUpdate(server, packet, client_channels, player1, player2, apple, 0, timeStart);
                }
                break;

            case 2:
                printf("Player 2 collided! Score1:%d Score2:%d\n", player1->score, player2->score);
                // Store score and decrement lives
                if (activeModifierPlayer2[4])
                {
                    int temp1s = player1->score;
                    int temp1l = player1->lives;
                    int temp2s = player2->score;
                    int temp2l = player2->lives;

                    InvincibleCollision(player2, &player1);

                    // Check if either player is out of lives
                    if (player1->lives <= 0 || player2->lives <= 0)
                    {
                        if (player1->lives <= 0 && player2->lives <= 0)
                        {
                            // Both players out of lives - determine winner by score
                            printf("Both players out of lives! ");
                            if (temp1s > temp2s)
                            {
                                printf("Player 1 wins by score!\n");
                            }
                            else if (temp1s < temp2s)
                            {
                                printf("Player 2 wins by score!\n");
                            }
                            else
                            {
                                printf("Game ended in a draw!\n");
                            }
                        }
                        else if (player1->lives <= 0)
                        {
                            printf("Player 1 out of lives! Player 2 wins!\n");
                        }
                        else
                        {
                            printf("Player 2 out of lives! Player 1 wins!\n");
                        }

                        // Send final game update
                        sendGameUpdate(server, packet, client_channels, player1, player2, apple, 1, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                        gameOver = 1;
                    }

                    if (player1->lives < temp1l)
                    {
                        // Player still has lives, reset snake position without ending game
                        freeSnake(player1);
                        player1 = createSnakePart(400, 420, RIGHT, 100, temp1l, temp1s, greenHead);
                        insertBodyPart(&player1, 380, 420, RIGHT, 100, temp1l, temp1s, green);
                        insertBodyPart(&player1, 360, 420, RIGHT, 100, temp1l, temp1s, green);
                        direction1 = RIGHT;
                        activeModifierPlayer1[4] = 0;
                        activeModifierPlayer1[3] = 0;
                        modifierTypePlayer1 = 0;
                        sendGameUpdate(server, packet, client_channels, player1, player2, apple, 0, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                    }
                    if (player2->lives < temp2l)
                    {
                        // Player still has lives, reset snake position without ending game
                        freeSnake(player2);
                        player2 = createSnakePart(400, 820, RIGHT, 100, temp2l, temp2s, blueHead);
                        insertBodyPart(&player2, 380, 820, RIGHT, 100, temp2l, temp2s, blue);
                        insertBodyPart(&player2, 360, 820, RIGHT, 100, temp2l, temp2s, blue);
                        direction2 = RIGHT;
                        activeModifierPlayer2[4] = 0;
                        activeModifierPlayer2[3] = 0;
                        modifierTypePlayer2 = 0;
                        sendGameUpdate(server, packet, client_channels, player1, player2, apple, 0, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                    }
                }
                else // Normal no invincibility
                {
                    int temp = player2->score;
                    int temp2 = --player2->lives;

                    // Check if player has lives left
                    if (player2->lives <= 0)
                    {
                        printf("Player 2 out of lives! Player 1 wins!\n");
                        // Send final game update with zero lives for player 2
                        sendGameUpdate(server, packet, client_channels, player1, player2, apple, 1, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                        gameOver = 1;
                        break;
                    }

                    // Player still has lives, reset snake position without ending game
                    freeSnake(player2);
                    player2 = createSnakePart(400, 820, RIGHT, 100, temp, temp2, blueHead);
                    insertBodyPart(&player2, 380, 820, RIGHT, 100, temp, temp2, blue);
                    insertBodyPart(&player2, 360, 820, RIGHT, 100, temp, temp2, blue);
                    direction2 = RIGHT;
                    activeModifierPlayer2[4] = 0;
                    activeModifierPlayer2[3] = 0;
                    modifierTypePlayer2 = 0;
                }
                // Send game update without game over flag
                // sendGameUpdate(server, packet, client_channels, player1, player2, apple, 0, timeStart);
                break;

            case 3:
                printf("Head-on collision! Score1:%d Score2:%d\n", player1->score, player2->score);

                if (activeModifierPlayer2[4] || activeModifierPlayer1[4])
                {
                    if (activeModifierPlayer2[4] && activeModifierPlayer1[4])
                    {
                        break;
                    }
                    else if (activeModifierPlayer1[4])
                    {
                        int temp = player2->score;
                        int temp2 = --player2->lives;

                        // Check if player has lives left
                        if (player2->lives <= 0)
                        {
                            printf("Player 2 out of lives! Player 1 wins!\n");
                            // Send final game update with zero lives for player 2
                            sendGameUpdate(server, packet, client_channels, player1, player2, apple, 1, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                            gameOver = 1;
                            break;
                        }

                        // Player still has lives, reset snake position without ending game
                        freeSnake(player2);
                        player2 = createSnakePart(400, 820, RIGHT, 100, temp, temp2, blueHead);
                        insertBodyPart(&player2, 380, 820, RIGHT, 100, temp, temp2, blue);
                        insertBodyPart(&player2, 360, 820, RIGHT, 100, temp, temp2, blue);
                        direction2 = RIGHT;
                        activeModifierPlayer2[4] = 0;
                        activeModifierPlayer2[3] = 0;
                        modifierTypePlayer2 = 0;
                    }
                    else
                    {
                        int temp = player1->score;
                        int temp2 = --player1->lives;

                        // Check if player has lives left
                        if (player1->lives <= 0)
                        {
                            printf("Player 1 out of lives! Player 2 wins!\n");
                            // Send final game update with zero lives for player 1
                            sendGameUpdate(server, packet, client_channels, player1, player2, apple, 1, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                            gameOver = 1;
                            break;
                        }

                        // Player still has lives, reset snake position without ending game
                        freeSnake(player1);
                        player1 = createSnakePart(400, 420, RIGHT, 100, temp, temp2, greenHead);
                        insertBodyPart(&player1, 380, 420, RIGHT, 100, temp, temp2, green);
                        insertBodyPart(&player1, 360, 420, RIGHT, 100, temp, temp2, green);
                        direction1 = RIGHT;
                        activeModifierPlayer1[4] = 0;
                        activeModifierPlayer1[3] = 0;
                        modifierTypePlayer1 = 0;
                    }
                }
                else
                {
                    // Store scores and decrement lives for both players
                    int temp1s = player1->score;
                    int temp1l = --player1->lives;
                    int temp2s = player2->score;
                    int temp2l = --player2->lives;

                    // Check if either player is out of lives
                    if (temp1l <= 0 || temp2l <= 0)
                    {
                        if (temp1l <= 0 && temp2l <= 0)
                        {
                            // Both players out of lives - determine winner by score
                            printf("Both players out of lives! ");
                            if (temp1s > temp2s)
                            {
                                printf("Player 1 wins by score!\n");
                            }
                            else if (temp1s < temp2s)
                            {
                                printf("Player 2 wins by score!\n");
                            }
                            else
                            {
                                printf("Game ended in a draw!\n");
                            }
                        }
                        else if (temp1l <= 0)
                        {
                            printf("Player 1 out of lives! Player 2 wins!\n");
                        }
                        else
                        {
                            printf("Player 2 out of lives! Player 1 wins!\n");
                        }

                        // Send final game update
                        sendGameUpdate(server, packet, client_channels, player1, player2, apple, 1, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                        gameOver = 1;

                        break;
                    }

                    // Both players still have lives, respawn both
                    freeSnake(player1);
                    freeSnake(player2);

                    player1 = createSnakePart(400, 420, RIGHT, 100, temp1s, temp1l, greenHead);
                    insertBodyPart(&player1, 380, 420, RIGHT, 100, temp1s, temp1l, green);
                    insertBodyPart(&player1, 360, 420, RIGHT, 100, temp1s, temp1l, green);

                    player2 = createSnakePart(400, 820, RIGHT, 100, temp2s, temp2l, blueHead);
                    insertBodyPart(&player2, 380, 820, RIGHT, 100, temp2s, temp2l, blue);
                    insertBodyPart(&player2, 360, 820, RIGHT, 100, temp2s, temp2l, blue);

                    direction1 = RIGHT;
                    direction2 = RIGHT;
                    activeModifierPlayer1[4] = 0;
                    activeModifierPlayer1[3] = 0;
                    activeModifierPlayer2[4] = 0;
                    activeModifierPlayer2[3] = 0;
                    modifierTypePlayer1 = 0;
                    modifierTypePlayer2 = 0;
                }
                // Send game update without game over flag
                // sendGameUpdate(server, packet, client_channels, player1, player2, apple, 0, timeStart);
                break;
            }

            if (isEaten(apple, player1))
            {
                Apple *temp = isEaten(apple, player1);

                if (temp)
                {
                    int appleType = temp->type; // Store apple type before deleting

                    removeApple(&apple, temp); // Remove eaten apple

                    switch (appleType)
                    {
                    case -1:
                        break;

                    case 0: // Regular apple, grow snake
                        generateApple(&apple, player1, player2, 0);
                        applesEatenFor_2++;
                        applesEatenFor_3++;
                        applesEatenFor_4++;
                        Snake *tail = getTail(player1);
                        if (tail)
                        {
                            int new_x = tail->x_coord;
                            int new_y = tail->y_coord;

                            // Determine new body part position
                            switch (tail->direction)
                            {
                            case UP:
                                new_y += CELL_SIZE;
                                break;
                            case RIGHT:
                                new_x -= CELL_SIZE;
                                break;
                            case DOWN:
                                new_y -= CELL_SIZE;
                                break;
                            case LEFT:
                                new_x += CELL_SIZE;
                                break;
                            }

                            insertBodyPart(&player1, new_x, new_y, tail->direction, tail->velocity, player1->score, player1->lives, tail->color);
                            player1->score++;
                        }
                        break;

                    case 1: // Special apple, remove last body part
                        generateApple(&apple, player1, player2, 1);

                        if (activeModifierPlayer1[4])
                        {
                            break;
                        }

                        int tempScore = --player1->score;
                        int tempLives = player1->lives;

                        tail = getTail(player1);
                        if (tail && tail != player1)
                        { // Ensure snake has more than one part
                            removeTail(&player1);
                        }
                        else
                        {

                            tempLives--;
                            // If no segments remain, check if lives are still available
                            if (tempLives <= 0)
                            {
                                printf("Player 1 out of lives! Player 2 wins!\n");
                                sendGameUpdate(server, packet, client_channels, player1, player2, apple, 1, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                                gameOver = 1;
                            }
                            else
                            {
                                printf("Player 1 lost all segments due to special apple! Resetting snake...\n");
                                player1 = createSnakePart(400, 420, RIGHT, 100, tempScore, tempLives, greenHead);
                                insertBodyPart(&player1, 380, 420, RIGHT, 100, tempScore, tempLives, green);
                                insertBodyPart(&player1, 360, 420, RIGHT, 100, tempScore, tempLives, green);
                                direction1 = RIGHT;
                            }
                        }

                        break;

                    case 2:

                        // generateApple(&apple, player1, player2, 2);

                        tail = getTail(player1);

                        for (int i = 0; i < 2; i++)
                        {
                            if (tail)
                            {
                                int new_x = tail->x_coord;
                                int new_y = tail->y_coord;

                                // Determine new body part position
                                switch (tail->direction)
                                {
                                case UP:
                                    new_y += CELL_SIZE;
                                    break;
                                case RIGHT:
                                    new_x -= CELL_SIZE;
                                    break;
                                case DOWN:
                                    new_y -= CELL_SIZE;
                                    break;
                                case LEFT:
                                    new_x += CELL_SIZE;
                                    break;
                                }

                                insertBodyPart(&player1, new_x, new_y, tail->direction, tail->velocity, player1->score, player1->lives, tail->color);
                                player1->score++;
                            }
                            tail = tail->next;
                        }

                        break;

                    case 3:
                        // generateApple(&apple, player1, player2, 3);

                        activeModifierPlayer1[3] = 1;
                        startTimeModifierPlayer1[3] = SDL_GetTicks();
                        player1->velocity = 60;
                        modifierTypePlayer1 = 3;
                        player1->score++;
                        break;

                    case 4:
                        // generateApple(&apple, player1, player2, 4);
                        totalModifierPlayer1[4]++;
                        player1->score += 7;
                        break;
                    }
                }
            }

            if (isEaten(apple, player2))
            {
                Apple *temp = isEaten(apple, player2);

                if (temp)
                {
                    int appleType = temp->type; // Store the apple type before deleting

                    removeApple(&apple, temp); // Remove the eaten apple from the linked list

                    switch (appleType)
                    {
                    case -1:
                        break;

                    case 0: // Regular apple, grow snake
                        generateApple(&apple, player1, player2, 0);
                        applesEatenFor_2++;
                        applesEatenFor_3++;
                        applesEatenFor_4++;
                        Snake *tail = getTail(player2);
                        int new_x = tail->x_coord;
                        int new_y = tail->y_coord;

                        // Determine new body part position based on tail's direction
                        switch (tail->direction)
                        {
                        case UP:
                            new_y += CELL_SIZE;
                            break;
                        case RIGHT:
                            new_x -= CELL_SIZE;
                            break;
                        case DOWN:
                            new_y -= CELL_SIZE;
                            break;
                        case LEFT:
                            new_x += CELL_SIZE;
                            break;
                        }

                        insertBodyPart(&player2, new_x, new_y, tail->direction, tail->velocity, player2->score, player2->lives, tail->color);
                        player2->score++;
                        break;

                    case 1: // Special apple, remove last body part
                        generateApple(&apple, player1, player2, 1);

                        if (activeModifierPlayer2[4])
                        {
                            break;
                        }
                        int tempScore = --player2->score;
                        int tempLives = player2->lives;

                        tail = getTail(player2);
                        if (tail && tail != player2)
                        {
                            removeTail(&player2); // Implement this function to properly remove tail
                        }
                        else
                        {
                            tempLives--;
                            // If no segments remain, check if lives are still available
                            if (tempLives <= 0)
                            {
                                printf("Player 2 out of lives! Player 1 wins!\n");
                                sendGameUpdate(server, packet, client_channels, player1, player2, apple, 1, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                                gameOver = 1;
                            }
                            else
                            {
                                printf("Player 2 lost all segments due to special apple! Resetting snake...\n");
                                player2 = createSnakePart(400, 820, RIGHT, 100, tempScore, tempLives, blueHead);
                                insertBodyPart(&player2, 380, 820, RIGHT, 100, tempScore, tempLives, blue);
                                insertBodyPart(&player2, 360, 820, RIGHT, 100, tempScore, tempLives, blue);
                                direction2 = RIGHT;
                            }
                        }
                        break;

                    case 2:
                        // generateApple(&apple, player1, player2, 2);

                        tail = getTail(player2);

                        for (int i = 0; i < 2; i++)
                        {
                            if (tail)
                            {
                                int new_x = tail->x_coord;
                                int new_y = tail->y_coord;

                                // Determine new body part position
                                switch (tail->direction)
                                {
                                case UP:
                                    new_y += CELL_SIZE;
                                    break;
                                case RIGHT:
                                    new_x -= CELL_SIZE;
                                    break;
                                case DOWN:
                                    new_y -= CELL_SIZE;
                                    break;
                                case LEFT:
                                    new_x += CELL_SIZE;
                                    break;
                                }

                                insertBodyPart(&player2, new_x, new_y, tail->direction, tail->velocity, player2->score, player2->lives, tail->color);
                                player2->score++;
                            }
                            tail = tail->next;
                        }

                        break;

                    case 3:
                        // generateApple(&apple, player1, player2, 3);

                        activeModifierPlayer2[3] = 1;
                        startTimeModifierPlayer2[3] = SDL_GetTicks();
                        player2->velocity = 60;
                        modifierTypePlayer2 = 3;
                        player2->score++;
                        break;

                    case 4:
                        // generateApple(&apple, player1, player2, 4);
                        totalModifierPlayer2[4]++;
                        player2->score += 7;
                        break;
                    }
                }
            }

            for (int i = 3; i < 10; i++)
            {
                if (activeModifierPlayer1[i])
                {
                    rightNowPlayer1[i] = SDL_GetTicks();
                    if (rightNowPlayer1[i] - startTimeModifierPlayer1[i] >= 5000)
                    {
                        activeModifierPlayer1[i] = 0;
                        if (i == 3)
                        {
                            player1->velocity = 100;
                            modifierTypePlayer1 = activeModifierPlayer1[4] ? 4 : 0;
                        }
                        if (i == 4)
                        {
                            modifierTypePlayer1 = activeModifierPlayer1[3] ? 3 : 0;
                        }
                    }
                }
            }

            for (int i = 3; i < 10; i++)
            {
                if (activeModifierPlayer2[i])
                {
                    rightNowPlayer2[i] = SDL_GetTicks();
                    if (rightNowPlayer2[i] - startTimeModifierPlayer2[i] >= 5000)
                    {
                        activeModifierPlayer2[i] = 0;
                        if (i == 3)
                        {
                            player2->velocity = 100;
                            modifierTypePlayer2 = activeModifierPlayer2[4] ? 4 : 0;
                        }
                        if (i == 4)
                        {
                            modifierTypePlayer2 = activeModifierPlayer2[3] ? 3 : 0;
                        }
                    }
                }
            }

            if (applesEatenFor_2 >= 8)
            {
                generateApple(&apple, player1, player2, 2);
                applesEatenFor_2 = 0;
            }

            if (applesEatenFor_3 >= 5)
            {
                generateApple(&apple, player1, player2, 3);
                applesEatenFor_3 = 0;
            }

            if (applesEatenFor_4 >= 15)
            {
                generateApple(&apple, player1, player2, 4);
                applesEatenFor_4 = 0; // Reset the counter after 10 apples.
            }

            // Update game state with fixed timing
            uint32_t currentTime = SDL_GetTicks();
            if (currentTime - lastMoveTime1 >= player1->velocity)
            {
                computerMove(player1, apple, player2);

                // moveSnake(player1, direction1);
                lastMoveTime1 = currentTime;
            }
            if (currentTime - lastMoveTime2 >= player2->velocity)
            {
                // computerMove(player2, apple, player1);
                moveSnake(player2, direction2);
                lastMoveTime2 = currentTime;
            }

            // Check if time has run out
            double timeRemaining = 180.0 - ((double)(SDL_GetTicks() - timeStart) / 1000.0);
            if (timeRemaining <= 0)
            {
                printf("Time's up! Final Score - Player 1: %d, Player 2: %d\n", player1->score, player2->score);
                sendGameOver(server, packet, client_channels, player1->score, player2->score);
                sendGameUpdate(server, packet, client_channels, player1, player2, apple, 1, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                gameOver = 1;
                break;
            }

            // Send game updates at a fixed interval (every 50ms)
            static uint32_t lastUpdateTime = 0;
            if (currentTime - lastUpdateTime >= 16)
            {
                sendGameUpdate(server, packet, client_channels, player1, player2, apple, gameOver, timeStart, modifierTypePlayer1, modifierTypePlayer2, soundEffect);
                lastUpdateTime = currentTime;
            }
        }
    }

    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(server);
    SDLNet_Quit();
    SDL_Quit();

    return 0;
}