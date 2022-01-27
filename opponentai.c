/**
 * @file opponentai.c
 * @author Maksim Kovalkov
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "util.h"
#include "tetris.h"

#include "opponentai.h"

/** 
 * Coefficients for the heuristic used to evaluate a particular move.
 * Metrics loosely inspired by open source AI projects for classic Tetris.
 * Values determined by guessing and common sense :)
 */
#define HEIGHT_COEFF  (-55.0)
#define LINES_COEFF   (+70.0)
#define HOLES_COEFF   (-35.0)
#define BUMPS_COEFF   (-40.0)
#define FUTURE_COEFF  (+0.90)
#define PENALTY_COEFF (+80.0)
#define RECURSE_DEPTH 1

struct Opponent_ai {
    int x, rots;
    int last_x;
    enum Tetrimino_type type;
    Board sim_board;
};

static double choose_best_move(Opponent_ai *, unsigned char const [7], int);
static double heuristic(Board const);

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
        /* ai->x = (rand() % 10) - 4;
        ai->rots = rand() % 3;
        ai->type = Tetrimino_type_I + (rand() % 7); */
        memcpy(ai->sim_board, game->board[1], sizeof ai->sim_board);
        ai->last_x = ai->x;
        choose_best_move(ai, game->pieces_left, RECURSE_DEPTH);
        return ai->type - Tetrimino_type_I + Game_action_Choose_I;
    case Game_state_Place:
        if (ai->rots) {
            --ai->rots;
            return Game_action_Rotate;
        }
        /* if we are not moving anymore, too bad: drop the piece and whatever happens happens */
        if (game->active_piece.x != ai->last_x) {
            if ((ai->last_x = game->active_piece.x) < ai->x)
                return Game_action_Right;
            else if (game->active_piece.x > ai->x)
                return Game_action_Left;
        }
        return Game_action_Drop;
    case Game_state_Cleared:
        return Game_action_Finish_clearing;
    case Game_state_Lose:
    case Game_state_Win:
        return Game_action_Queue_empty;
    }
}

double
heuristic(Board const board)
{
    double heu;
    int heights[BOARD_COLS];
    int i, j, max_height = 0, lines = 0, holes = 0, bumps = 0;

    /* compute heights per column */
    for (j = 0; j < BOARD_COLS; ++j) {
        for (i = 0; i < BOARD_ROWS && !board[i][j]; ++i) /* nop */;
        heights[j] = BOARD_ROWS - i;
    }

    /* find max height */
    for (i = 0; i < BOARD_COLS; ++i)
        if (heights[i] > max_height)
            max_height = heights[i];

    /* count full lines */
    for (i = 0; i < BOARD_ROWS; ++i) {
        int full = 1;
        for (j = 0; full && j < BOARD_COLS; ++j) {
            if (!board[i][j])
                full = 0;
        }
        lines += full;
    }

    /* count holes */
    for (j = 0; j < BOARD_COLS; ++j) {
        for (i = BOARD_ROWS - heights[j] + 1; i < BOARD_ROWS; ++i) {
            if (!board[i][j])
                ++holes;
        }
    }

    /* count "bumps" (height difference between adjacent columns) */
    for (i = 1; i < BOARD_COLS; ++i)
        bumps += abs(heights[i] - heights[i-1]);

    /* return final heuristic */
    heu = HEIGHT_COEFF * max_height + LINES_COEFF * lines + PENALTY_COEFF * (lines >= 3)
            + HOLES_COEFF * holes + BUMPS_COEFF * bumps;
    /* fprintf(stderr, "h=%d l=%d o=%d b=%d\theu=%.3lf\n", max_height, lines, holes, bumps, heu); */
    return heu;
}

double
choose_best_move(Opponent_ai *ai, unsigned char const pieces_left[7], int depth) {
    Piece piece;
    double score, max_score = -1e20;

    /* try every piece type, in every rotation, at every available column */    
    for (piece.type = Tetrimino_type_I; piece.type <= Tetrimino_type_O; ++piece.type) {
        int rots;
        if (pieces_left[piece.type-1] == 0)
            continue;

        init_piece_shape(&piece);
        for (rots = 0; rots < 4; ++rots) {
            lift_piece(&piece, ai->sim_board);
            for (piece.x = -2; piece.x + 2 < BOARD_COLS; ++piece.x) {
                if (collides(&piece, ai->sim_board))
                    continue;
                drop_piece(&piece, ai->sim_board);
                place_piece(&piece, ai->sim_board, piece.type);

                /* fprintf(stderr, "(%d, %d, %d) -> \t\t", piece.type, rots, piece.x); */
                score = heuristic(ai->sim_board);
                /* recursive call with board state that includes the current piece:
                   take into account the next step's best move in our calculations */
                if (depth > 0)
                    score += FUTURE_COEFF * choose_best_move(ai, pieces_left, depth-1);
                /* reset board state to previous condition */
                place_piece(&piece, ai->sim_board, Block_type_Empty);
                if (score > max_score) {
                    max_score = score;
                    ai->x = piece.x;
                    ai->rots = rots;
                    ai->type = piece.type; 
                }
            }
            rotate_shape_cw(piece.shape);
        }
    }
    return max_score;
}
