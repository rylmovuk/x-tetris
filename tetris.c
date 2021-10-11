#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define BOARD_ROWS 15
#define BOARD_COLS 10

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
 * Each one's blocks are already set to its own "type".
 * (relying on the literal values of `enum tetrimino_types` :/ but whatever)
 * Indices are <value of `enum tetrimino_type`> - 1.
 */
typedef unsigned char TetriminoShape[4][4];
static TetriminoShape tetrimino_shapes[] = {
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
    },
    {
        {0, 2, 0, 0},
        {0, 2, 2, 0},
        {0, 2, 0, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 3, 0},
        {0, 0, 3, 0},
        {0, 3, 3, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 4, 0, 0},
        {0, 4, 0, 0},
        {0, 4, 4, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 5, 0, 0},
        {0, 5, 5, 0},
        {0, 0, 5, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 6, 0},
        {0, 6, 6, 0},
        {0, 6, 0, 0},
        {0, 0, 0, 0},
    },
    {
        {0, 0, 0, 0},
        {0, 7, 7, 0},
        {0, 7, 7, 0},
        {0, 0, 0, 0},
    }
};

/**
 * Struct representing a piece "that we care about", containing its shape and coordinates.
 * Note that a valid state *can* contain x and/or y that are out of bounds relative to the game board:
 * but in that case any cell shape[i][j] *must* be 0 if board[y+i][x+j] is out of bounds.
 */
typedef struct Piece {
    unsigned char type;
    int y, x;
    TetriminoShape shape;
} Piece;

/* Convenience function to set the shape of a piece. */
void init_piece_shape(Piece *p) {
    memcpy(&p->shape, tetrimino_shapes[p->type-1], sizeof p->shape);
}

void rotate_shape_cw(TetriminoShape shape) {
    /* 
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

/* String constants for drawing the frame. */
static char *    frame_top = "┏━━━━━━━━━━━━━━━━━━━━┓";
static char * frame_bottom = "┗━━━━━━━━━━━━━━━━━━━━┛";
static char * frame_border = "┃";

/* Strings each block inside the game board gets translated to. */
static const char * const block_types[] = {
    /* BLOCK_EMPTY -> */ "  ",
    /* TETRIMINO_I -> */ "░░",
    /* TETRIMINO_T -> */ "▒▒",
    /* TETRIMINO_J -> */ "▓▓",
    /* TETRIMINO_L -> */ "██",
    /* TETRIMINO_S -> */ "░░",
    /* TETRIMINO_Z -> */ "▒▒",
    /* TETRIMINO_O -> */ "▓▓",
    /* BLOCK_GHOST -> */ "⣏⣹",
    /* BLOCK_CLEAR -> */ "⣶⣶",
    /* BLOCK_BADBK -> */ "‼︎‼︎"
};

/**
 * Format used to show the number of pieces left when letting the player choose the next piece.
 * Sorry for all the string concatenaton jank -- wanted the source code to look aligned like the end result.
 */
static const char *piece_choice_prompt =
    " █ x%-2d  █▄ x%-2d  █▀ x%-2d  █  x%-2d  █▄ x%-2d  ▄█ x%-2d  ██ x%-2d\n"
    " █ <i> "" ▀  <t> "" ▀  <j> "" ▀▀ <l> ""  ▀ <s> "" ▀  <z> ""    <o>\n";

static const char *movement_prompt =
    "<h> move left    <l> move right\n"
    "    <r> rotate       <j> drop\n";


enum game_state {
    /* The player must choose the next piece. */
    GAME_STATE_CHOOSE,
    /* The player is moving the piece to the desired location. */
    GAME_STATE_PLACE,
    /* One or more lines have been cleared -- show it to the player before removing them */
    GAME_STATE_CLEARED,

    GAME_STATE_LOSE,
    GAME_STATE_WIN
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
    unsigned char pieces_left[7];
} Game;

/**
 * Place a piece on the board with no "collision" checking and assuming it fits inside the bounds.
 * Note that x and y can actually be negative, and likewise y + 4 can be >= BOARD_WIDTH. It is the caller's
 * responsibility to ensure that no block compised by the piece ends up out of bounds -- in other words,
 * looking at piece->shape, **only zeroes** can end up outside of the board.
 */
void place_piece(const Piece *p, Board board) {
    int i, j;
    unsigned char bk;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            if ((bk = p->shape[i][j])) board[p->y + i][p->x + j] = bk;
        }
    }
}

/**
 * Similar to `place_piece`, but instead of copying over the (non-empty) blocks from the piece, 
 * sets the board to `bk` in the shape of `piece`.
 * Can be used to "cut out" a piece by filling with empty space or to draw "ghost pieces".
 * Having both variants feels kind of useless :/ but whatever
 */
void set_as_piece(const Piece *p, Board board, unsigned char bk) {
    int i, j;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4;++j) {
            if (p->shape[i][j]) board[p->y + i][p->x + j] = bk;
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
    while (1) {
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
    piece->y = 0;
    do {
        --piece->y;
    } while (!collides(piece, board));
    ++piece->y;
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
int mark_cleared_lines(Game *game) {
    int i, j, cleared_count = 0;

    for (i = 0; i < BOARD_ROWS; ++i) {
        int cleared = 1;
        for (j = 0; j < BOARD_COLS; ++j)
            if (!game->board[i][j]) cleared = 0;

        if (cleared) {
            memset(game->board[i], BLOCK_CLEAR, BOARD_COLS);
            ++cleared_count;
        }
    }

    return cleared_count;
}

/**
 * Removes every cleared line previously marked by `mark_cleared_lines` (more accurately, where the *first* block is
 * set to `BLOCK_CLEAR`). Shifts everything downwards.
 */
void remove_cleared_lines(Game *game) {
    int i = BOARD_ROWS - 1;

    while (i > 0) {
        if (game->board[i][0] != BLOCK_CLEAR) {
            --i;
            continue;
        }
        
        if (i != 0)
            /* multidimensional arrays *should* be contiguous in memory so this *should* be valid
               please daddy no undefined behaviour :,) */
            memmove(
                ((unsigned char *)game->board) + BOARD_COLS,
                (unsigned char *)game->board,
                i * BOARD_COLS
            );
        memset(game->board[0], BLOCK_EMPTY, sizeof game->board[0]);
    }
}

void draw(Game *game) {
    int i, j;
    Piece ghost;

    if (game->state == GAME_STATE_PLACE) {
        memcpy(&ghost, &game->active_piece, sizeof ghost);
        drop_piece(&ghost, game->board);
        if (ghost.y - game->active_piece.y >= 3)
            place_piece(&game->active_piece, game->board);
        set_as_piece(&ghost, game->board, BLOCK_GHOST);
    } else if (game->state == GAME_STATE_LOSE) {
        set_as_piece(&game->active_piece, game->board, BLOCK_BADBK);
    }

    puts(frame_top);
    for (i = 0; i < BOARD_ROWS; ++i) {
        fputs(frame_border, stdout);
        for (j = 0; j < BOARD_COLS; ++j) {
            fputs(block_types[game->board[i][j]], stdout);
        }
        puts(frame_border);
    }
    puts(frame_bottom);

    switch (game->state) {
    case GAME_STATE_PLACE:
        set_as_piece(&game->active_piece, game->board, BLOCK_EMPTY);
        set_as_piece(&ghost, game->board, BLOCK_EMPTY);

        printf(movement_prompt, 0);
        break;
    case GAME_STATE_CHOOSE:
        printf(piece_choice_prompt, game->pieces_left[0], game->pieces_left[1], game->pieces_left[2],
              game->pieces_left[3], game->pieces_left[4], game->pieces_left[5], game->pieces_left[6]);
        break;
    case GAME_STATE_CLEARED:
        puts("nice!");
        break;
    case GAME_STATE_LOSE:
        puts("...game over!");
        break;
    case GAME_STATE_WIN:
        puts("congratulations!!!");
        break;
    }

}

void game_loop(Game *game) {
    char c;

    while (game->state != GAME_STATE_LOSE && game->state != GAME_STATE_WIN) {
        draw(game);

        do { scanf("%c", &c); } while (isspace(c));
        switch (game->state) {
        case GAME_STATE_PLACE:
            {
                int cleared_lines;
                switch (c) {
                case 'h':
                    handle_left(game);
                    break;
                case 'l':
                    handle_right(game);
                    break;
                case 'r':
                    handle_rotate(game);
                    break;
                case 'j':
                    drop_piece(&game->active_piece, game->board);
                    place_piece(&game->active_piece, game->board);

                    cleared_lines = mark_cleared_lines(game);
                    if (cleared_lines > 0)
                        game->state = GAME_STATE_CLEARED;
                    else
                        game->state = GAME_STATE_CHOOSE;
                    break;
                default: continue;
                }
            }
            break;
        case GAME_STATE_CHOOSE:
            {
                unsigned char t;
                for (t = 0; t < 7; ++t)
                    if (game->pieces_left[t]) break;
                /* if 0 of each piece is left */
                if (t == 7) {
                    game->state = GAME_STATE_WIN;
                    break;
                }
                switch (c) {
                case 'i': t = TETRIMINO_I; break;
                case 't': t = TETRIMINO_T; break;
                case 'j': t = TETRIMINO_J; break;
                case 'l': t = TETRIMINO_L; break;
                case 's': t = TETRIMINO_S; break;
                case 'z': t = TETRIMINO_Z; break;
                case 'o': t = TETRIMINO_O; break;
                default: continue;
                }
                if (game->pieces_left[t-1] <= 0) continue;
                --game->pieces_left[t-1];

                game->active_piece.type = t;
                init_piece_shape(&game->active_piece);
                game->active_piece.x = 3;
                lift_piece(&game->active_piece, game->board);
                if (collides(&game->active_piece, game->board)) 
                    game->state = GAME_STATE_LOSE;
                else
                    game->state = GAME_STATE_PLACE;
            }
            break;
        case GAME_STATE_CLEARED:
            {
                remove_cleared_lines(game);
                game->state = GAME_STATE_CHOOSE;
            }
            break;
        case GAME_STATE_LOSE: 
        case GAME_STATE_WIN: ; /* loop condition */
        }
    }

    /* draw one last time after the player lost */
    draw(game);
}

int main() {
    Game game = {
        /* state = */ GAME_STATE_CHOOSE,
        /* board = */ {
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {1, 2, 0, 0, 0, 3, 3, 3, 0, 0},
            {1, 2, 2, 5, 5, 0, 4, 3, 7, 7},
            {1, 2, 5, 5, 4, 4, 4, 0, 7, 7},
        },
        /* active_piece = */ {0,},
        /* pieces_left = */ {20, 20, 20, 20, 20, 20, 20},
    };

    game_loop(&game);

    return 0;
}
