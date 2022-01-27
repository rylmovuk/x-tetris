/**
 * @file opponentai.h
 * @author Maksim Kovalkov
 */
#ifndef XTETRIS_OPPONENTAI_H
#define XTETRIS_OPPONENTAI_H

#include "tetris.h"

typedef struct Opponent_ai Opponent_ai;

/**
 * Allocate and initialize an Opponent_ai. Exits on failure.  
 * Caller owns the returned object and must call `ai_destroy` to correctly clean up.  
 * @returns a pointer to a valid instance of Opponent_ai.
 */
Opponent_ai * ai_create(void);
/**
 * Deinitialize and deallocate an Opponent_ai, which must have been initialized by `ai_create`.  
 */
void ai_destroy(Opponent_ai *);

enum Game_action ai_next_action(Opponent_ai *, Game const *);


#endif /* ifndef XTETRIS_OPPONENTAI_H */
