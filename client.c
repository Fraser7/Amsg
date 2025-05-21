#include "display.h"

#define BUFFER_SIZE 10000
UDPsocket client;
IPaddress server_addr;
UDPpacket *packet;
int isGameOver = 0;
int myPlayerID = -1;
int modifierTypePlayer1;
int modifierTypePlayer2;
double timeRemaining = 180.0;

void loadSoundEffects(SoundEffects *soundEffect){
    soundEffect[0].effect = Mix_LoadWAV("sound/bruh.wav");
    soundEffect[0].duration = 1000;

    soundEffect[1].effect = Mix_LoadWAV("sound/eating.wav");
    soundEffect[1].duration = 2000;

    soundEffect[2].effect = Mix_LoadWAV("sound/nani.wav");
    soundEffect[2].duration = 5000;

    soundEffect[3].effect = Mix_LoadWAV("sound/really_nigga.wav");
    soundEffect[3].duration = 1500;

    soundEffect[4].effect = Mix_LoadWAV("sound/roblox_death.wav");
    soundEffect[4].duration = 1000;

    soundEffect[5].effect = Mix_LoadWAV("sound/wasted.wav");
    soundEffect[5].duration = 4000;
    
    soundEffect[6].effect = Mix_LoadWAV("sound/wow.wav");
    soundEffect[6].duration = 2000;

    for(int i = 0; i < 7; i++){
        if(!soundEffect[i].effect){
            printf("Failed to load sound %d: %s\n", i, Mix_GetError());
        }
    }
}


// Function to send input (WASD) to the server
void sendInput(char input)
{
    packet->data[0] = input;
    packet->len = sizeof(char);
    packet->address = server_addr;

    if (SDLNet_UDP_Send(client, -1, packet) == 0)
    {
        printf("Failed to send input: %s\n", SDLNet_GetError());
    }
}

// Function to update local snake copy
void updateSnake(Snake **player, int x[], int y[], int length, int score, int lives, int PlayerID)
{
    // printf("Started updating snake %d\n", PlayerID + 1);
    SDL_Color green = {0, 255, 0, 255};
    SDL_Color blue = {0, 0, 255, 255};
    SDL_Color greenHead = {0, 150, 0, 255};
    SDL_Color blueHead = {0, 0, 150, 255};
    SDL_Color bodyColor = (PlayerID == 0) ? green : blue;
    SDL_Color headColor = (PlayerID == 0) ? greenHead : blueHead;

    Snake *current = *player;
    Snake *prev = NULL;
    int i = 0;

    // Update existing nodes
    while (current != NULL && i < length)
    {
        current->x_coord = x[i];
        current->y_coord = y[i];
        current->score = score;
        current->lives = lives;
        current->color = (i == 0) ? headColor : bodyColor;

        prev = current;
        current = current->next;
        i++;
    }
    // printf("Updated %d nodes\n", i);
    //  If we need more nodes, create them
    while (i < length)
    {
        Snake *newPart = malloc(sizeof(Snake));
        if (!newPart)
        {
            printf("Memory allocation failed for new snake part\n");
            return;
        }

        newPart->x_coord = x[i];
        newPart->y_coord = y[i];
        newPart->score = score;
        newPart->lives = lives;
        newPart->color = (i == 0) ? headColor : bodyColor;
        newPart->next = NULL;

        if (prev == NULL)
        {
            *player = newPart;
        }
        else
        {
            prev->next = newPart;
        }
        prev = newPart;
        i++;
    }
    // printf("Added %d new nodes\n", i - length);

    // Free extra nodes if the new length is smaller than the old length
    if (current != NULL)
    {
        if (prev != NULL)
        {
            prev->next = NULL; // Ensure the last valid node doesn't point to freed memory
        }

        int freedNodes = 0;
        while (current != NULL)
        {
            Snake *next = current->next;
            free(current);
            current = next;
            freedNodes++;
        }
        // printf("Freed %d extra nodes\n", freedNodes);
    }
}

void updateApple(Apple **apple, int apple_x[], int apple_y[], int apple_type[], int apple_count)
{
    Apple *current = *apple, *temp;

    // Free old apple list
    while (current)
    {
        temp = current->next;
        free(current);
        current = temp;
    }
    *apple = NULL;

    Apple *head = NULL, *tail = NULL;
    for (int i = 0; i < apple_count; i++)
    {
        Apple *newApple = malloc(sizeof(Apple));
        if (!newApple)
        {
            printf("Memory allocation failed for Apple %d\n", i);
            break;
        }

        newApple->x_coord = apple_x[i];
        newApple->y_coord = apple_y[i];
        newApple->type = apple_type[i];
        newApple->next = NULL;

        if (!head)
        {
            head = newApple;
        }
        else
        {
            tail->next = newApple;
        }
        tail = newApple;
    }
    *apple = head;
}

// Function to receive game updates from the server
void receiveGameUpdate(Snake **player1, Snake **player2, Apple **apple, int *soundEffect)
{
    int received = SDLNet_UDP_Recv(client, packet);
    if (received == 0)
    {
        // No data received, this is normal, don't exit
        return;
    }
    if (received < 0)
    {
        printf("Error receiving data: %s\n", SDLNet_GetError());
        return;
    }

    // Check if this is a game over packet (only contains gameOver flag)
    if (received == sizeof(int))
    {
        int gameOver;
        memcpy(&gameOver, packet->data, sizeof(int));
        if (gameOver == 1)
        {
            printf("Game Over! Exiting...\n");
            isGameOver = 1;
            return;
        }
    }

    int offset = 0;
    // Regular game update packet
    int gameOver;
    memcpy(&gameOver, packet->data + offset, sizeof(int));
    offset += sizeof(int);

    if (gameOver == 1)
    {
        printf("Game Over signaled by server!\n");
        isGameOver = 1;
    }
    memcpy(soundEffect, packet->data + offset, sizeof(int));
    offset += sizeof(int);

    int length1, length2, apple_length;
    int lives1, lives2, score1, score2;
    memcpy(&apple_length, packet->data + offset, sizeof(int));
    offset += sizeof(int);
    int *apple_x = (int *)malloc(apple_length * sizeof(int));
    int *apple_y = (int *)malloc(apple_length * sizeof(int));
    int *apple_type = (int *)malloc(apple_length * sizeof(int));
    memcpy(apple_x, packet->data + offset, apple_length * sizeof(int));
    offset += apple_length * sizeof(int);
    memcpy(apple_y, packet->data + offset, apple_length * sizeof(int));
    offset += apple_length * sizeof(int);
    memcpy(apple_type, packet->data + offset, apple_length * sizeof(int));
    offset += apple_length * sizeof(int);

    memcpy(&length1, packet->data + offset, sizeof(int));
    offset += sizeof(int);
    int *x1 = (int *)malloc(length1 * sizeof(int));
    int *y1 = (int *)malloc(length1 * sizeof(int));
    memcpy(x1, packet->data + offset, length1 * sizeof(int));
    offset += length1 * sizeof(int);
    memcpy(y1, packet->data + offset, length1 * sizeof(int));
    offset += length1 * sizeof(int);

    memcpy(&length2, packet->data + offset, sizeof(int));
    offset += sizeof(int);
    int *x2 = (int *)malloc(length2 * sizeof(int));
    int *y2 = (int *)malloc(length2 * sizeof(int));
    memcpy(x2, packet->data + offset, length2 * sizeof(int));
    offset += length2 * sizeof(int);
    memcpy(y2, packet->data + offset, length2 * sizeof(int));
    offset += length2 * sizeof(int);

    memcpy(&score1, packet->data + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&score2, packet->data + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&lives1, packet->data + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&lives2, packet->data + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&modifierTypePlayer1, packet->data + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&modifierTypePlayer2, packet->data + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&timeRemaining, packet->data + offset, sizeof(double));
    offset += sizeof(double);

    memcpy(&myPlayerID, packet->data + offset, sizeof(int));
    offset += sizeof(int);
    // printf("Packets decoded\n");

    // Always update with the fixed order from the packet:
    updateSnake(player1, x1, y1, length1, score1, lives1, 0);
    updateSnake(player2, x2, y2, length2, score2, lives2, 1);
    // printf("Both snakes updated\n");

    // If myPlayerID==1, swap the pointers so that localSnake points to the blue snake.
    if (myPlayerID == 1)
    {
        Snake *temp = *player1;
        *player1 = *player2;
        *player2 = temp;

        int temp1 = modifierTypePlayer1;
        modifierTypePlayer1 = modifierTypePlayer2;
        modifierTypePlayer2 = temp1;

        // Now, *player1 is your local snake (blue) and *player2 is the opponent (green).
    }
    free(x1);
    free(y1);
    free(x2);
    free(y2);
    free(apple_x);
    free(apple_y);
    free(apple_type);
    // printf("Snakes updated\n");
    updateApple(apple, apple_x, apple_y, apple_type, apple_length);
    // printf("Apples updated\n");
    //  Display game status for debugging
    if (*player1 && *player2)
    {
        printf("Player status - You: Lives %d, Score %d | Opponent: Lives %d, Score %d | Time: %.2f\n",
               (*player1)->lives, (*player1)->score, (*player2)->lives, (*player2)->score, timeRemaining);
    }
}

int main(int argc, char *argv[])
{
    // if (argc != 2)
    // {
    //     printf("Usage: %s <server_ip>\n", argv[0]);
    //     printf("Example: %s 192.168.1.100\n", argv[0]);
    //     return 1;
    // }

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    if (SDL_Init(SDL_INIT_AUDIO) < 0){
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1)
    {
        printf("TTF could not initialize! TTF_Error: %s\n", TTF_GetError());
        return 1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    Mix_Music *backgroundMusic = Mix_LoadMUS("sound/kirby.mp3");
    if(!backgroundMusic)
    {
        printf("Failed to load background music! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("Nunito.ttf", 28);
    
    if (!font)
    {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        return 1;
    }
    
    if (SDLNet_Init() < 0)
    {
        printf("Failed to initialize SDL_net: %s\n", SDLNet_GetError());
        return 1;
    }


    SoundEffects soundManager[10];
    loadSoundEffects(soundManager);
    int soundEffect;
    
    printf("Enter server IP address: ");
    char server_ip[16];
    scanf("%s", server_ip);

    
    
    client = SDLNet_UDP_Open(0);
    if (!client)
    {
        printf("Failed to open UDP socket: %s\n", SDLNet_GetError());
        return 1;
    }
    
    
    if (SDLNet_ResolveHost(&server_addr, server_ip, 8888) < 0)
    {
        printf("Failed to resolve server address: %s\n", SDLNet_GetError());
        return 1;
    }

    printf("Connecting to server at %s:8888\n", argv[1]);
    
    packet = SDLNet_AllocPacket(BUFFER_SIZE);
    if (!packet)
    {
        printf("Failed to allocate packet: %s\n", SDLNet_GetError());
        return 1;
    }
    
    SDL_Color green = {0, 255, 0, 255};
    SDL_Color blue = {0, 0, 255, 255};
    SDL_Color greenHead = {0, 100, 0, 255}; // Much darker green for head
    SDL_Color blueHead = {0, 0, 100, 255};  // Much darker blue for head
    
    Snake *player1 = NULL;
    Snake *player2 = NULL;
    Apple *apple = NULL;
    SDL_Event event;
    
    // Reduce initial delay to 100ms
    SDL_Delay(100);
    
    // Send initial connection packet
    char connection_packet = 'C'; // 'C' for connection
    packet->data[0] = connection_packet;
    packet->len = sizeof(char);
    packet->address = server_addr;
    SDLNet_UDP_Send(client, -1, packet);
    
    // Wait for initial game state
    printf("Waiting for game state...\n");
    while (player1 == NULL || player2 == NULL || apple == NULL)
    {
        receiveGameUpdate(&player1, &player2, &apple, &soundEffect);
        SDL_Delay(10); // Small delay to prevent CPU overuse
    }
    printf("Game state received! Starting game...\n");
    SDL_Window *window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_RaiseWindow(window);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    Mix_PlayMusic(backgroundMusic, -1); // Play background music in a loop
    Mix_VolumeMusic(90);

    while (!isGameOver)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                printf("Game Over from sdl quit! Exiting...\n");
                isGameOver = 1;
                break;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_w:
                    sendInput('W');
                    break;
                case SDLK_a:
                    sendInput('A');
                    break;
                case SDLK_s:
                    sendInput('S');
                    break;
                case SDLK_d:
                    sendInput('D');
                    break;
                case SDLK_i:
                    sendInput('I');
                    break;
                case SDLK_ESCAPE:
                    isGameOver = 1;
                    break;
                default:
                    break;
                }
            }
        }
        // printf("Polling ho gayi\n");
        receiveGameUpdate(&player1, &player2, &apple, &soundEffect);
        // printf("Update aa gayo\n");
        if (player1 == NULL || player2 == NULL)
        {
            printf("Lost connection to game state, waiting for reconnection...\n");
            SDL_Delay(100); // Longer delay when waiting for reconnection
            continue;
        }

        SDL_SetRenderDrawColor(renderer, 53, 62, 67, 255);
        SDL_RenderClear(renderer);

        if(!Mix_Playing(2)){
            Mix_PlayChannel(2, soundManager[soundEffect].effect, 0);
        }
        

        drawGrid(renderer);

        drawSnake(player1, renderer, modifierTypePlayer1);
        drawSnake(player2, renderer, modifierTypePlayer2);

        drawApple(apple, renderer);

        renderScore(renderer, font, player1, player2, timeRemaining);
        
        SDL_RenderPresent(renderer);
    }

    printf("Game Over! Exiting...\n");
    Mix_FreeMusic(backgroundMusic);
    SDLNet_FreePacket(packet);
    SDLNet_Quit();
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
