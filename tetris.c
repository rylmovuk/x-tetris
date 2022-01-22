#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "tetris.h"
#include "iohandler.h"
#include "graphics.h"

/* ------ Function prototypes ------ */

static void init_piece_shape(Piece *);
static void rotate_shape_cw(Tetrimino_shape);
static void place_piece(const Piece *, Board, unsigned char);
static int collides(const Piece *, const Board);
static void drop_piece(Piece *, const Board);
static void lift_piece(Piece *, const Board);
static void handle_right(Game *);
static void handle_left(Game *);
static void handle_rotate(Game *);
static int check_win_condition(Game *);
static int mark_cleared_lines(Board);
static void remove_cleared_lines(Board);
static int do_game_step(Game *, enum Game_action);
static void draw(Game *, Io_handler *);
static void atexit_fn(void);

static void state_place_handler(Game *, enum Game_action);
static void state_choose_handler(Game *, enum Game_action);
static void state_cleared_handler(Game *);

static int action_belongs_to_state(enum Game_action, enum Game_state);

static void game_init(Game *);
static void game_loop(Game *, Io_handler *);

/* ------ Static data ------ */
/**
 * Representation of each tetrimino (in one of the possible rotations) in a 
 * Indices are <value of `enum Tetrimino_type`> - 1.
 */
static const Tetrimino_shape tetrimino_shapes[] = {
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
    },
    {
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
    }
};

Io_handler *g_io_handler = NULL;

/* ------ Functions ------ */

/**
 * Convenience function to set the shape of a piece during initialization, by reading the `type` field.
 */
void
init_piece_shape(Piece *p)
{
    memcpy(&p->shape, tetrimino_shapes[p->type-1], sizeof p->shape);
}

/**
 * Rotate a shape clockwise.
 */
void
rotate_shape_cw(Tetrimino_shape shape)
{
    /* 
     * Dumb but it works :P
     *
     * (0,0) (0,1) (0,2) (0,3)      (3,0) (2,0) (1,0) (0,0)
     * (1,0) (1,1) (1,2) (1,3)  ->  (3,1) (2,1) (1,1) (0,1)
     * (2,0) (2,1) (2,2) (2,3)      (3,2) (2,2) (1,2) (0,2)
     * (3,0) (3,1) (3,2) (3,3)      (3,3) (2,3) (1,3) (0,3)
     */
    int i;
    unsigned char t;

    /* rotate outer ring */
    for (i = 0; i < 3; ++i) {
        t = shape[0][i];
        shape[0][i] = shape[3-i][0];
        shape[3-i][0] = shape[3][3-i];
        shape[3][3-i] = shape[i][3];
        shape[i][3] = t;
    }
    /* rotate inner ring */
    t = shape[1][1];
    shape[1][1] = shape[2][1];
    shape[2][1] = shape[2][2];
    shape[2][2] = shape[1][2];
    shape[1][2] = t;
}

/**
 * Place a piece on the board with no "collision" checking and assuming it fits inside the bounds,
 * setting each cell occupied by the piece to `type`.
 * Can be used to "cut out" a piece by filling with empty space or to draw "ghost pieces".
 * Note that x and y can actually be negative, and likewise y + 4 can be >= BOARD_WIDTH. It is the caller's
 * responsibility to ensure that no block compised by the piece ends up out of bounds -- in other words,
 * looking at piece->shape, **only zeroes** can end up outside of the board.
 */
void
place_piece(const Piece *p, Board board, unsigned char type)
{
    int i, j;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            if (p->shape[i][j]) board[p->y + i][p->x + j] = type;
        }
    }
}

/**
 * Check if the piece, when placed, would collide with existing blocks or with the outer bounds of the board.
 */
int
collides(const Piece *p, const Board board)
{
    int i, j;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            /* only looking at the coordinates of actual blocks that form the piece */
            if (!p->shape[i][j]) continue;
            /* if the block collides with the outside frame */
            if (p->y + i < 0 || p->y + i >= BOARD_ROWS
             || p->x + j < 0 || p->x + j >= BOARD_COLS)
               return 1;
            /* if the block collides with another block */
            if (board[p->y + i][p->x + j]) return 1;
        }
    }
    return 0;
}

/**
 * Drop the piece, in the conventional Tetris sense. More precisely, this means keeping its x value, and setting
 * its y value to put it as low as possible on the board without colliding with other pieces.
 */
void
drop_piece(Piece *piece, const Board board)
{
    for (;;) {
        ++piece->y;
        if (collides(piece, board)) {
            --piece->y;
            return;
        }
    }
}

/**
 * Leave a piece's x value unchanged, and set its y value so that the piece is in the topmost position on the screen.
 */
void
lift_piece(Piece *piece, const Board board)
{
    int i, j;
    piece->y = 0;
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 4; ++j)
            if (piece->shape[i][j]) return;
        --piece->y;
    }
}

/**
 * Move the active piece right by one, if possible.
 */
void
handle_right(Game *game)
{
    ++game->active_piece.x;
    if (collides(&game->active_piece, game->board))
        --game->active_piece.x;
}

/**
 * Move the active piece left by one, if possible.
 */
void
handle_left(Game *game)
{
    --game->active_piece.x;
    if (collides(&game->active_piece, game->board))
        ++game->active_piece.x;
}

/**
 * Rotate the active piece, dealing with possible collisions.
 */
void
handle_rotate(Game *game)
{
    rotate_shape_cw(game->active_piece.shape);
    lift_piece(&game->active_piece, game->board);

    /* we have to adjust the piece if the rotation brings some of its blocks out of bounds */
    if (game->active_piece.x < 0)
        while (collides(&game->active_piece, game->board)) ++game->active_piece.x;
    else if (game->active_piece.x + 4 >= BOARD_COLS)
        while (collides(&game->active_piece, game->board)) --game->active_piece.x;
}

/**
 * Set each full line on the game board to `Block_type_Clear`.
 * @returns the amount of lines to be cleared.
 */
int
mark_cleared_lines(Board board)
{
    int i, j, cleared_count = 0;

    for (i = 0; i < BOARD_ROWS; ++i) {
        int cleared = 1;
        for (j = 0; j < BOARD_COLS; ++j)
            if (!board[i][j]) cleared = 0;

        if (cleared) {
            memset(board[i], Block_type_Clear, BOARD_COLS);
            ++cleared_count;
        }
    }

    return cleared_count;
}

/**
 * Remove every cleared line previously marked by `mark_cleared_lines` (more accurately, where at least
 * the *first* block is set to `Block_type_Clear`). Shift everything downwards.
 */
void
remove_cleared_lines(Board board)
{
    int i = BOARD_ROWS - 1;

    while (i > 0) {
        if (board[i][0] != Block_type_Clear) {
            --i;
            continue;
        }
        
        if (i != 0)
            /* multidimensional arrays are contiguous in memory so this is valid */
            memmove(
                ((unsigned char *)board) + BOARD_COLS,
                (unsigned char *)board,
                i * BOARD_COLS
            );
        memset(board[0], Block_type_Empty, sizeof board[0]);
    }
}
/**
 * Check whether the player has won, i.e. has used all of their pieces.
 */
int
check_win_condition(Game *game)
{
    int i;
    for (i = 0; i < 7; ++i) 
        if (game->pieces_left[i] != 0) return 0;
    return 1;
}

/**
 * Prepare the game state for drawing (possibly breaking invariants assumed elsewhere in the game logic!),
 * send the state to the I/O function for display, then restore everything to its original value.
 */
void
draw(Game *game, Io_handler *io_handler)
{
    Piece ghost;

    /* prepare board state */
    if (game->state == Game_state_Place) {
        memcpy(&ghost, &game->active_piece, sizeof ghost);
        drop_piece(&ghost, game->board);
        if (ghost.y - game->active_piece.y >= 3)
            place_piece(&game->active_piece, game->board, game->active_piece.type);
        place_piece(&ghost, game->board, Block_type_Ghost);
    } else if (game->state == Game_state_Lose) {
        place_piece(&game->active_piece, game->board, Block_type_Badbk);
    }

    iohandler_draw_1p(io_handler, game);

    /* done drawing */
    fflush(stdout);

    /* clean up board state */
    if (game->state == Game_state_Place) {
        place_piece(&game->active_piece, game->board, Block_type_Empty);
        place_piece(&ghost, game->board, Block_type_Empty);
    }
}

/**
 * Helper to handle actions for `Game_state_Place`.  
 * `act` must be an appropriate action for this state.
 */
void
state_place_handler(Game *game, enum Game_action act)
{
    switch (act) {
    case Game_action_Left:
        handle_left(game);
        break;
    case Game_action_Right:
        handle_right(game);
        break;
    case Game_action_Rotate:
        handle_rotate(game);
        break;
    case Game_action_Drop:
        drop_piece(&game->active_piece, game->board);
        place_piece(&game->active_piece, game->board, game->active_piece.type);

        game->lines_cleared = mark_cleared_lines(game->board);
        if (game->lines_cleared) {
            game->state = Game_state_Cleared;
        } else {
            game->state = check_win_condition(game) ? Game_state_Win : Game_state_Choose;
        }
        break;
    default: break;
    /* exhaustive */
    }
}

/**
 * Helper to handle actions for `Game_state_Choose`.  
 * `act` must be an appropriate action for this state.
 */
void
state_choose_handler(Game *game, enum Game_action act)
{
    unsigned char t = act - Game_action_Choose_I + Tetrimino_type_I;

    if (game->pieces_left[t-1] <= 0) return;
    --game->pieces_left[t-1];

    game->active_piece.type = t;
    init_piece_shape(&game->active_piece);
    /* place in the middle */
    game->active_piece.x = BOARD_COLS / 2 - 2;
    lift_piece(&game->active_piece, game->board);
    if (collides(&game->active_piece, game->board)) {
        game->state = Game_state_Lose;
    } else {
        game->state = Game_state_Place;
    }
}

/**
 * Helper to transition from the intermediate state when some lines have just been cleared.
 */
void
state_cleared_handler(Game *game)
{
    remove_cleared_lines(game->board);
    game->score += score_per_lines[game->lines_cleared - 1];

    game->lines_cleared = 0;
    game->state = check_win_condition(game) ? Game_state_Win : Game_state_Choose;
}

/**
 * Given one of the game actions as received from IO, advances the game state as needed.
 * Assumes the action given is coherent with the game state (the IO handler must ensure that!).
 * The return value determines whether other actions can be chained after the current one.
 * @returns 1 if the function can be called again in the same game loop iteration, otherwise 0.
 */
int
do_game_step(Game *game, enum Game_action act)
{
    if (act == Game_action_Queue_empty)
        return 0;

    assert(action_belongs_to_state(act, game->state));

    switch (game->state) {
    case Game_state_Choose:
        state_choose_handler(game, act);
        return 1;
    case Game_state_Place:
        state_place_handler(game, act);
        /* can chain move&rotate actions; have to pause for the drop action */
        return act != Game_action_Drop;
    case Game_state_Cleared:
        state_cleared_handler(game);
        /* fallthrough */
    case Game_state_Lose:
    case Game_state_Win:
        return 0;
    }
}

/**
 * Check that a particular action can be executed in the given state.  
 * This function is just for extra peace of mind and to simplify debugging.
 */
int
action_belongs_to_state(enum Game_action act, enum Game_state state)
{
    return ((act & 0xe0) == (state << 5));
}

/**
 * Run the game: keep executing the game loop until the game ends.
 */
void
game_loop(Game *game, Io_handler *io_handler)
{
    /* draw, then handle as many actions as we can */
    for (;;) {
        draw(game, io_handler);

        if (game->state == Game_state_Win || game->state == Game_state_Lose)
            break;

        while (do_game_step(game, iohandler_next_action_1p(io_handler, game))) /* nop */;
    }
}

/**
 * Set the game state to the initial configuration.
 */
void
game_init(Game *game)
{
    game->state = Game_state_Choose;
    memset(game->board, 0, BOARD_COLS * BOARD_ROWS);

    /* game->active_piece = uninitialized  -- chosen later */
    game->score = 0;
    memset(game->pieces_left, STARTING_PIECES, sizeof game->pieces_left);
    game->lines_cleared = 0;
}

void
atexit_fn()
{
    if (g_io_handler)
        iohandler_destroy(g_io_handler);
}

int
main()
{
    Game game;
    g_io_handler = iohandler_create();
    atexit(&atexit_fn);

    game_init(&game);
    game_loop(&game, g_io_handler);

    return EXIT_SUCCESS;
}
