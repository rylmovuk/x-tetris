#ifndef XTETRIS_IOHANDLER_H
#define XTETRIS_IOHANDLER_H

#include "tetris.h"

typedef struct Io_handler Io_handler;

/**
 * Allocate and initialize an Io_handler. Exits on failure.  
 * Caller owns the returned object and must call `iohandler_destroy` to correctly clean up.  
 * @returns a pointer to a valid instance of Io_handler.
 */
Io_handler * iohandler_create();
/**
 * Deinitialize and deallocate an Io_handler, which must have been initialized by `iohandler_create`.  
 */
void iohandler_destroy(Io_handler *);

/**
 * Given the current game state, process one input and return a game action coherent with the game state.  
 * This function is called multiple times in a single game loop iteration. It must keep yielding actions until 
 * there are any valid inputs, then return a special value `Game_action_Queue_empty` to signal that there are 
 * no more actions.
 */
enum Game_action iohandler_next_action_1p(Io_handler *, Game const *);
/* (?) enum Game_action iohandler_next_action_2p(Io_handler *, Game const *); */

/**
 * Given a game state prepared for drawing, update the visual representation of the board; 
 * reset the input handler state, since inputs cannot be carried over from a previous game loop iteration.
 */
void iohandler_draw_1p(Io_handler *, Game const *);
/* (?) void iohandler_draw_2p(Io_handler *, Game *); */
#endif /* ifndef XTETRIS_IOHANDLER_H */
