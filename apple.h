#ifndef APPLE_H_
#define APPLE_H_

#include "gamestate.h"

void generateApple(Apple** apple, Snake *player1, Snake *player2, int type);
Apple* isEaten(Apple *apple, Snake *head);
void removeApple(Apple **apple, Apple *toDelete);

#endif // APPLE_H_

/*
type 0 - normal apple [red] :)
type 1 - -1 body [green] :)
type 2 - +2 body [blue] :)
type 3 - speed increase [yellow] :)
type 4 - invincibility shield  [purple] :)
type 5 - invisiblity with less speed and limited vision [black]
type 6 - cannon  [orange]
type 7 - walls hacks [grey]
type 8 - multiple copies hack [pink]
type 9 - time rewind 5s [black and white]
type 10 - splits in half (one is AI controlled) [cyan]
type 11 - head->tail and tail->head [green and blue]
type 12 - moving apple [red and yellow]
type 13 - reverse opponents controls [brown]
type 14 - snapshot (can get that times score and abilities back) [white]
*/
