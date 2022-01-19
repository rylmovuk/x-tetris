#ifndef XTETRIS_IOHANDLER_H
#define XTETRIS_IOHANDLER_H

#include "tetris.h"

typedef struct Io_handler Io_handler;

Io_handler * iohandler_create();
void iohandler_destroy(Io_handler *);

enum Game_action iohandler_next_action_1p(Io_handler *, Game *);
/* (?) enum Game_action iohandler_next_action_2p(Io_handler *, Game const *); */

void iohandler_draw_1p(Io_handler *, Game *);
/* (?) void iohandler_draw_2p(Io_handler *, Game *); */
#endif /* ifndef XTETRIS_IOHANDLER_H */
