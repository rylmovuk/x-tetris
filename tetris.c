#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define BOARD_ROWS 15
#define BOARD_COLS 10
#define STARTING_PIECES 20

#include "graphics.h"

#pragma region Types
/**
 * Enum to represent the possible types of tetrimino, as well as the type ("color", like in the "standardized" versions
 * of Tetris) of each block on the game board (when interpreted as a `enum block_type`).
 * 0 is not used as it represents "empty space".
 */
enum tetrimino_type {
    TETRIMINO_I = 1,
    TETRIMINO_T = 2,
    TETRIMINO_J = 3,
    TETRIMINO_L = 4,
    TETRIMINO_S = 5,
    TETRIMINO_Z = 6,
    TETRIMINO_O = 7
};

/**
 * Enum to represent types of block that show up when the game board is printed. This includes the tetrimino types,
 * which are not redefined here (use `enum tetrimino_type`).
 */
enum block_type {
    BLOCK_EMPTY = 0,
    /* tetrimino block types --> 1 ... 7 */
    BLOCK_GHOST = 8,
    BLOCK_CLEAR = 9,
    BLOCK_BADBK = 10
};

/**
 * Representation of each tetromino (in one of the possible rotations) in a 4x4 grid.
 * Indices are <value of `enum tetrimino_type`> - 1.
 */
typedef unsigned char TetriminoShape[4][4];

/**
 * Struct representing a piece "that we care about", containing its shape and coordinates.
 * Note that a valid state *can* contain x and/or y that are out of bounds relative to the game board:
 * but in that case any cell at `shape[i][j]` *must* be 0 if `board[y+i][x+j]` is out of bounds.
 */
typedef struct Piece {
    unsigned char type;
    int y, x;
    TetriminoShape shape;
} Piece;

enum game_state {
    /* The player must choose the next piece. */
    GAME_STATE_CHOOSE = 0,
    /* The player is moving the piece to the desired location. */
    GAME_STATE_PLACE = 1,

    GAME_STATE_LOSE = 2,
    GAME_STATE_WIN = 3,

    /* One or more lines have been cleared -- show it to the player before removing them */
    GAME_STATE_CLEARED = 4
};
/**
 * Representation of the playing field.
 * Sadly for now its meaning is overloaded as it represents both the logical state of the board (presence or absence
 * of a block in a given cell) and the visual state ("color" of each block; "ghost" blocks) :/ but whatever
 */
typedef unsigned char Board[BOARD_ROWS][BOARD_COLS];

/* Struct representing the entire game state. */
typedef struct Game {
    enum game_state state;
    Board board;
    Piece active_piece;
    int score;
    unsigned char pieces_left[7];
    int lines_cleared;
} Game;

/**
 * A function to handle one input from a sequence of inputs for a given game state.
 * Returns 1 the screen needs to be redrawn after the current input. The rest of the sequence is discarded.
 * Returns 0 if we can go on to the next input without redrawing -- even if the state has changed.
 */
typedef int (InputHandlerFn)(Game *, char);
#pragma endregion Types

#pragma region Func_prototypes
static void init_piece_shape(Piece *);
static void rotate_shape_cw(TetriminoShape);
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
static void board_draw_line(Board, int);
static void ui_draw_line(Game *, int);
static void draw(Game *);

static InputHandlerFn state_place_handler,
               state_choose_handler,
               state_cleared_handler,
               do_nothing_handler;

static void game_init(Game *game);
static void game_loop(Game *game);

#pragma endregion Func_prototypes

#pragma region Static_data
static const TetriminoShape tetrimino_shapes[] = {
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

/* Points that get added to the score for clearing 1, 2, 3 or 4 lines respectively. */
static const int score_per_lines[] = {1, 3, 6, 12};

static InputHandlerFn *input_handlers[] = {
    /*  GAME_STATE_CHOOSE -> */ state_choose_handler,
    /*   GAME_STATE_PLACE -> */ state_place_handler,
    /*    GAME_STATE_LOSE -> */ do_nothing_handler,
    /*     GAME_STATE_WIN -> */ do_nothing_handler,
    /* GAME_STATE_CLEARED -> */ state_cleared_handler,
};
#pragma endregion Static_data

/* Convenience function to set the shape of a piece. */
void init_piece_shape(Piece *p) {
    memcpy(&p->shape, tetrimino_shapes[p->type-1], sizeof p->shape);
}

void rotate_shape_cw(TetriminoShape shape) {
    /* 
     * Dumb but it works :P
     *
     * (0,0) (0,1) (0,2) (0,3)      (3,0) (2,0) (1,0) (0,0)
     * (1,0) (1,1) (1,2) (1,3)  ->  (3,1) (2,1) (1,1) (0,1)
     * (2,0) (2,1) (2,2) (2,3)      (3,2) (2,2) (1,2) (0,2)
     * (3,0) (3,1) (3,2) (3,3)      (3,3) (2,3) (1,3) (0,3)
     */
    int i, j;
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
void place_piece(const Piece *p, Board board, unsigned char type) {
    int i, j;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            if (p->shape[i][j]) board[p->y + i][p->x + j] = type;
        }
    }
}

/**
 * Checks if the piece, when placed, would collide with existing blocks or with the outer bounds of the board.
 */
int collides(const Piece *p, const Board board) {
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
 * Drops the piece, in the conventional Tetris sense. More precisely, this means keeping its x value, and setting
 * its y value to put it as low as possible on the board without colliding with other pieces.
 */
void drop_piece(Piece *piece, const Board board) {
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
void lift_piece(Piece *piece, const Board board) {
    int i, j;
    piece->y = 0;
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 4; ++j)
            if (piece->shape[i][j]) return;
        --piece->y;
    }
}

void handle_right(Game *game) {
    ++game->active_piece.x;
    if (collides(&game->active_piece, game->board))
        --game->active_piece.x;
}

void handle_left(Game *game) {
    --game->active_piece.x;
    if (collides(&game->active_piece, game->board))
        ++game->active_piece.x;
}

void handle_rotate(Game *game) {
    rotate_shape_cw(game->active_piece.shape);
    lift_piece(&game->active_piece, game->board);

    /* we have to adjust the piece if the rotation brings some of its blocks out of bounds */
    if (game->active_piece.x < 0)
        while (collides(&game->active_piece, game->board)) ++game->active_piece.x;
    else if (game->active_piece.x + 4 >= BOARD_COLS)
        while (collides(&game->active_piece, game->board)) --game->active_piece.x;
}

/**
 * Sets each full line on the game board to `BLOCK_CLEAR`.
 */
int mark_cleared_lines(Board board) {
    int i, j, cleared_count = 0;

    for (i = 0; i < BOARD_ROWS; ++i) {
        int cleared = 1;
        for (j = 0; j < BOARD_COLS; ++j)
            if (!board[i][j]) cleared = 0;

        if (cleared) {
            memset(board[i], BLOCK_CLEAR, BOARD_COLS);
            ++cleared_count;
        }
    }

    return cleared_count;
}

/**
 * Removes every cleared line previously marked by `mark_cleared_lines` (more accurately, where at least
 * the *first* block is set to `BLOCK_CLEAR`). Shifts everything downwards.
 */
void remove_cleared_lines(Board board) {
    int i = BOARD_ROWS - 1;

    while (i > 0) {
        if (board[i][0] != BLOCK_CLEAR) {
            --i;
            continue;
        }
        
        if (i != 0)
            /* multidimensional arrays *should* be contiguous in memory so this *should* be valid
               please daddy no undefined behaviour :,) */
            memmove(
                ((unsigned char *)board) + BOARD_COLS,
                (unsigned char *)board,
                i * BOARD_COLS
            );
        memset(board[0], BLOCK_EMPTY, sizeof board[0]);
    }
}
/**
 * Returns whether the player has won, i.e. has used all of their pieces.
 */
int check_win_condition(Game *game) {
    int i;
    for (i = 0; i < 7; ++i) 
        if (game->pieces_left[i] != 0) return 0;
    return 1;
}

void board_draw_line(Board board, int line) {
    int col;

    if (line == 0) {
        fputs(frame_top, stdout);
    } else if (line == BOARD_ROWS+1) {
        fputs(frame_bottom, stdout);
    } else {
        --line; /* line 0 used to be the border; now we look at the lines inside the board */
        fputs(frame_border, stdout);
        for (col = 0; col < BOARD_COLS; ++col) {
            fputs(block_types[board[line][col]], stdout);
        }
        fputs(frame_border, stdout);
    }
}

void ui_draw_line(Game *game, int line) {
    /* here we render the UI on the side of the board
        we do this ugly and opaque dance because we render everything directly to stdout, line by line,
        without relying on terminal capabilities. */
    switch (line) {
    case 3: /* -- display score */
        printf(interface[line], game->score);
        break;
    case 6: /* -- message line 1 */
        /* lines_cleared should be 0 whenever, after the last dropped piece, no lines were cleared */
        printf(interface[line], messages[game->state + game->lines_cleared][0]);
        break;
    case 7: /* -- message line 2 */
        printf(interface[line], messages[game->state + game->lines_cleared][1]);
        break;
    case 11: /* -- pieces left: I T J L */
        printf(interface[line], game->pieces_left[0], game->pieces_left[1], game->pieces_left[2], game->pieces_left[3]); 
        break;
    case 12: /* -- piece keys: i t j l */
        if (game->state == GAME_STATE_CHOOSE)
            printf(interface[line], piece_keys[0], piece_keys[1], piece_keys[2], piece_keys[3]);
        else
            printf(interface[line], "", "", "", "");
        break;
    case 14: /* -- pieces left: S Z O */
        printf(interface[line], game->pieces_left[4], game->pieces_left[5], game->pieces_left[6]);
        break;
    case 15: /* -- piece keys: s z o */
        if (game->state == GAME_STATE_CHOOSE)
            printf(interface[line], piece_keys[4], piece_keys[5], piece_keys[6]);
        else
            printf(interface[line], "", "", "");
        break;
    default: /* every other line is to be printed as-is */
        fputs(interface[line], stdout);
    }
}

void draw(Game *game) {
    int i, j;
    Piece ghost;

    /* prepare board state */
    if (game->state == GAME_STATE_PLACE) {
        memcpy(&ghost, &game->active_piece, sizeof ghost);
        drop_piece(&ghost, game->board);
        if (ghost.y - game->active_piece.y >= 3)
            place_piece(&game->active_piece, game->board, game->active_piece.type);
        place_piece(&ghost, game->board, BLOCK_GHOST);
    } else if (game->state == GAME_STATE_LOSE) {
        place_piece(&game->active_piece, game->board, BLOCK_BADBK);
    }

    for (i = 0; i < BOARD_ROWS+2; ++i) {
        board_draw_line(game->board, i);
        fputs("  ", stdout);
        ui_draw_line(game, i);
        
        putchar('\n');
    }

    fputs(prompt[game->state], stdout);

    /* done drawing */
    fflush(stdout);

    /* clean up board state */
    if (game->state == GAME_STATE_PLACE) {
        place_piece(&game->active_piece, game->board, BLOCK_EMPTY);
        place_piece(&ghost, game->board, BLOCK_EMPTY);
    }
}

int state_place_handler(Game *game, char c) {
    switch (c) {
    case 'H':
    case 'h':
        handle_left(game);
        break;
    case 'L':
    case 'l':
        handle_right(game);
        break;
    case 'r':
    case 'R':
        handle_rotate(game);
        break;
    case 'j':
    case 'J':
        drop_piece(&game->active_piece, game->board);
        place_piece(&game->active_piece, game->board, game->active_piece.type);

        game->lines_cleared = mark_cleared_lines(game->board);
        if (game->lines_cleared) {
            game->state = GAME_STATE_CLEARED;
        } else {
            game->state = check_win_condition(game) ? GAME_STATE_WIN : GAME_STATE_CHOOSE;
        }
        return 1;
    default:
        /* unrecognized character -- stop handling input */
        return 1;
    case ' ': ; /* whitespace to separate inputs is ok, like ' hh rrrr j' */
    }
    return 0;
}

int state_choose_handler(Game *game, char c) {
    unsigned char t;
    switch (c) {
    case 'i': case 'I': t = TETRIMINO_I; break;
    case 't': case 'T': t = TETRIMINO_T; break;
    case 'j': case 'J': t = TETRIMINO_J; break;
    case 'l': case 'L': t = TETRIMINO_L; break;
    case 's': case 'S': t = TETRIMINO_S; break;
    case 'z': case 'Z': t = TETRIMINO_Z; break;
    case 'o': case 'O': t = TETRIMINO_O; break;
    default: /* invalid input */ return 1;
    }
    if (game->pieces_left[t-1] <= 0) return 1;
    --game->pieces_left[t-1];

    game->active_piece.type = t;
    init_piece_shape(&game->active_piece);
    /* place in the middle */
    game->active_piece.x = 3;
    lift_piece(&game->active_piece, game->board);
    if (collides(&game->active_piece, game->board)) {
        game->state = GAME_STATE_LOSE;
        return 1;
    } else {
        game->state = GAME_STATE_PLACE;
        /* allow chaining the choice of a piece with movement controls, like 'trhhj'*/
        return 0;
    }
}

int state_cleared_handler(Game *game, char c) { 
    int i;

    remove_cleared_lines(game->board);
    game->score += score_per_lines[game->lines_cleared - 1];

    game->lines_cleared = 0;
    game->state = check_win_condition(game) ? GAME_STATE_WIN : GAME_STATE_CHOOSE;
    /* redraw in any case */
    return 1;
}
int do_nothing_handler(Game *game, char c) {
    return 1;
}

void game_loop(Game *game) {
    /* stop reading at 32 chars -- who would chain so many inputs anyway? */
    const int limit = 32;

    while (game->state != GAME_STATE_WIN && game->state != GAME_STATE_LOSE) {
        int c, n_read = 0;

        draw(game);

        /* read 1 char at a time, stop at EOF or when we read too many chars */
        while ((c = getchar()) != EOF && n_read++ < limit) {
            /* if the line is empty (we get a \n right away), we still call the handler once */
            if (input_handlers[game->state](game, c) || c == '\n') break;
        }

        /* skip to end of line if we stopped reading before newline */
        while (c != EOF && c != '\n')
            c = getchar();
    }   
    draw(game);
}

void game_init(Game *game) {
    /* most field are initializable to zero */
    memset(game, 0, sizeof *game);

    game->state = GAME_STATE_CHOOSE;
    memset(game->pieces_left, STARTING_PIECES, sizeof game->pieces_left);
}

int main() {
    Game game;

    /* since we redraw everything at once, buffering is very useful for performance */
    setvbuf(stdout, NULL, _IOFBF, 1024);

    game_init(&game);
    game_loop(&game);

    return 0;
}
