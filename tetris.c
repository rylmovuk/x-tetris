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
 * used to representwhich are not redefined here (use `enum tetrimino_type`).
 */
enum block_type {
    BLOCK_EMPTY = 0,
    /* tetrimino block types --> 1 ... 7 */
    BLOCK_GHOST = 8
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
void init_piece_shape(Piece *p, enum tetrimino_type t) {
    p->type = t;
    memcpy(&p->shape, tetrimino_shapes[t-1], sizeof p->shape);
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
    /* BLOCK_GHOST -> */ "⣏⣹"
};

/**
 * Representation of the playing field.
 * Sadly for now its meaning is overloaded as it represents both the logical state of the board (presence or absence
 * of a block in a given cell) and the visual state ("color" of each block; "ghost" blocks) :/ but whatever
 */
typedef unsigned char Board[BOARD_ROWS][BOARD_COLS];

/* Struct representing the entire game state. */
typedef struct Game {
    Board board;
    Piece active_piece;
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

void draw(Game *game) {
    int i, j;
    Piece ghost;

    if (game->active_piece.type) {
        memcpy(&ghost, &game->active_piece, sizeof ghost);
        drop_piece(&ghost, game->board);
        place_piece(&game->active_piece, game->board);
        set_as_piece(&ghost, game->board, BLOCK_GHOST);
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

    if (game->active_piece.type) {
        set_as_piece(&game->active_piece, game->board, BLOCK_EMPTY);
        set_as_piece(&ghost, game->board, BLOCK_EMPTY);
    }

}

void game_loop(Game *game) {
    char c;

    do {
        draw(game);

        scanf("%c", &c);
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
            game->active_piece.type = 0;
            break;
        default:
            continue;
        }
    } while (c != 'j');
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

    init_piece_shape(&game.active_piece, TETRIMINO_T);
    rotate_shape_cw(game.active_piece.shape);
    game.active_piece.y = -1; game.active_piece.x = 3;

    game_loop(&game);

    draw(&game);

    return 0;
}
