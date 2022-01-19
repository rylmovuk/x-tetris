#ifndef XTETRIS_TETRIS_H
#define XTETRIS_TETRIS_H
#include "constants.h"

/**
 * Enum to represent the possible types of tetrimino, as well as the type ("color", like in the "standardized" versions
 * of Tetris) of each block on the game board (when interpreted as a `enum block_type`).
 * 0 is not used as it represents "empty space".
 */
enum tetrimino_type {
    TETRIMINO_I = 1,
    TETRIMINO_T,
    TETRIMINO_J,
    TETRIMINO_L,
    TETRIMINO_S,
    TETRIMINO_Z,
    TETRIMINO_O
};

/**
 * Enum to represent types of block that show up when the game board is printed. This includes the tetrimino types,
 * which are not redefined here (use `enum tetrimino_type`).
 */
enum block_type {
    BLOCK_EMPTY = 0,
    /* tetrimino block types --> 1 ... 7 */
    BLOCK_GHOST = 8,
    BLOCK_CLEAR,
    BLOCK_BADBK
};

/**
 * 4x4 grid to represent each tetrimino shape.
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
    Game_state_choose,
    /* The player is moving the piece to the desired location. */
    Game_state_place,

    Game_state_lose,
    Game_state_win,

    /* One or more lines have been cleared -- show it to the player before removing them */
    Game_state_cleared
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

enum Game_action {
    Game_action_left,
    Game_action_right,
    Game_action_rotate,
    Game_action_place,
    Game_action_choose_i,
    Game_action_choose_t,
    Game_action_choose_j,
    Game_action_choose_l,
    Game_action_choose_s,
    Game_action_choose_z,
    Game_action_choose_o
};
#endif /* ifndef XTETRIS_TETRIS_H */
