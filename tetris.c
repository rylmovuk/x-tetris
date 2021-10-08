#include <stdio.h>
#include <string.h>

#define BOARD_ROWS 15
#define BOARD_COLS 10

/*
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

/* Enum to represent types of block that show up when the game board is printed. This includes the tetrimino types,
 * used to representwhich are not redefined here (use `enum tetrimino_type`).
 */
enum block_type {
    BLOCK_EMPTY = 0,
    /* tetrimino block types --> 1 ... 7 */
    BLOCK_GHOST = 8
};

/*
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

typedef struct Piece {
    TetriminoShape shape;
} Piece;

void init_piece(Piece *p, enum tetrimino_type t) {
    memcpy(&p->shape, tetrimino_shapes[t-1], sizeof p->shape);
}

void rotate_piece_cw(Piece *p) {
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
        t = p->shape[0][i];
        p->shape[0][i] = p->shape[3-i][0];
        p->shape[3-i][0] = p->shape[3][3-i];
        p->shape[3][3-i] = p->shape[i][3];
        p->shape[i][3] = t;
    }
    /* rotate inner ring */
    t = p->shape[1][1];
    p->shape[1][1] = p->shape[2][1];
    p->shape[2][1] = p->shape[2][2];
    p->shape[2][2] = p->shape[1][2];
    p->shape[1][2] = t;
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
    /* BLOCK_GHOST -> */ "⣏⣹"
};

/*
 * Representation of the playing field.
 * Sadly for now its meaning is overloaded as it represents both the logical state of the board (presence or absence
 * of a block in a given cell) and the visual state ("color" of each block; "ghost" blocks) :/ but whatever
 */
typedef unsigned char Board[BOARD_ROWS][BOARD_COLS];

/* Struct representing the entire game state. */
typedef struct Game {
    Board board;
} Game;

/*
 * Place a piece on the board with no "collision" checking and assuming it fits inside the bounds.
 * Note that x and y can actually be negative, and likewise y + 4 can be >= BOARD_WIDTH. It is the caller's
 * responsibility to ensure that no block compised by the piece ends up out of bounds -- in other words,
 * looking at piece->shape, **only zeroes** can end up outside of the board.
 */
void _place_piece(Board board, const Piece * piece, int y, int x) {
    int i, j, bk;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            if ((bk = piece->shape[i][j])) board[y+i][x+j] = bk;
        }
    }
}

void draw(const Game * game) {
    int i, j;

    puts(frame_top);
    for (i = 0; i < BOARD_ROWS; ++i) {
        fputs(frame_border, stdout);
        for (j = 0; j < BOARD_COLS; ++j) {
            fputs(block_types[game->board[i][j]], stdout);
        }
        puts(frame_border);
    }
    puts(frame_bottom);

}

int main() {
    Game game = {
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
        }
    };
    
    Piece tee;

    init_piece(&tee, TETRIMINO_T);
    rotate_piece_cw(&tee);

    _place_piece(game.board, &tee, 3, 5);
    _place_piece(game.board, &tee, -1, -1);

    draw(&game);

    return 0;
}
