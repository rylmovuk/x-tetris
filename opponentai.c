#include <stdlib.h>
#include <time.h>

#include "util.h"
#include "tetris.h"

#include "opponentai.h"

struct Opponent_ai {
    int x, r;
};

Opponent_ai *
ai_create()
{
    Opponent_ai *ai = malloc_or_die(sizeof (Opponent_ai));

    srand((unsigned) time(NULL));
    return ai;
}

void
ai_destroy(Opponent_ai *ai)
{
    free(ai);
}

enum Game_action
ai_next_action(Opponent_ai *ai, Game const *game)
{
    switch (game->state) {
    case Game_state_Choose:
        ai->x = (rand() % 10) - 4;
        ai->r = rand() % 3;
        return Game_action_Choose_I + (rand() % 7);
    case Game_state_Place:
        if (ai->x < 0) {
            ++ai->x;
            return Game_action_Right;
        } else if (ai->x > 0) {
            --ai->x;
            return Game_action_Left;
        } else if (ai->r) {
            --ai->r;
            return Game_action_Rotate;
        } else {
            return Game_action_Drop;
        }
    case Game_state_Cleared:
        return Game_action_Finish_clearing;
    case Game_state_Lose:
    case Game_state_Win:
        return Game_action_Queue_empty;
    }
}
