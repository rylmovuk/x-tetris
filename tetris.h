#ifndef XTETRIS_TETRIS_H
#define XTETRIS_TETRIS_H
#include "constants.h"

/**
 * Enum to represent the possible types of tetrimino, as well as the type ("color", like in the "standardized" versions
 * of Tetris) of each block on the game board (when interpreted as a `enum Block_type`).
 * 0 is not used as it represents "empty space".
 */
enum Tetrimino_type {
    Tetrimino_type_I = 1,
    Tetrimino_type_T,
    Tetrimino_type_J,
    Tetrimino_type_L,
    Tetrimino_type_S,
    Tetrimino_type_Z,
    Tetrimino_type_O
};

/**
 * Enum to represent types of block that show up when the game board is printed. This includes the tetrimino types,
 * which are not redefined here (use `enum Tetrimino_type`).
 */
enum Block_type {
    Block_type_Empty = 0,
    /* tetrimino block types --> 1 ... 7 */
    Block_type_Ghost = 8,
    Block_type_Clear,
    Block_type_Badbk
};

/**
 * 4x4 grid to represent each tetrimino shape.
 */
typedef unsigned char Tetrimino_shape[4][4];

/**
 * Struct representing a piece "that we care about", containing its shape and coordinates.
 * Note that a valid state *can* contain x and/or y that are out of bounds relative to the game board:
 * but in that case any cell at `shape[i][j]` *must* be 0 if `board[y+i][x+j]` is out of bounds.
 */
typedef struct Piece {
    unsigned char type;
    int y, x;
    Tetrimino_shape shape;
} Piece;

/**
 * One of the possible named states the game can be in.
 */
enum Game_state {
    /* The player must choose the next piece. */
    Game_state_Choose = 1,
    /* The player is moving the piece to the desired location. */
    Game_state_Place,

    Game_state_Lose,
    Game_state_Win,

    /* One or more lines have been cleared -- show it to the player before removing them */
    Game_state_Cleared
};

enum Game_kind {
    Game_kind_Singleplayer,
    Game_kind_Vs_player,
    Game_kind_Vs_ai
};

/**
 * Representation of the playing field.  
 * Does not only represent the presence or absence of a block: the meaning of the cells is related to (but still distinct
 * from) the visual presentation the user sees in the end.  
 * - Normally it includes the "type" ("color") of each block.  
 * - When some lines are cleared, the blocks in these lines are marked, and removed only on the next game update.
 * - In the draw stage, the board includes the active piece at the top, and the "ghost" piece at the bottom.
 */
typedef unsigned char Board[BOARD_ROWS][BOARD_COLS];

/** 
 * Struct representing the entire game state.
 */
typedef struct Game {
    enum Game_kind kind;
    enum Game_state state;
    Board board[2];
    Piece active_piece;
    int score[2];
    unsigned char pieces_left[7];
    int lines_cleared;
    int current_player;
} Game;

/**
 * A player action that can be performed.  An Io_handler transforms valid player inputs into these actions.  
 * Each action is only valid for a certain state.  To check that the action is coherent with the state
 * (for debugging purposes), use `(game_action & 0xe0) == (game_state << 5)`.
 */
enum Game_action {
    Game_action_Queue_empty = 0,

    Game_action_Choose_I = Game_state_Choose << 5,
    Game_action_Choose_T,
    Game_action_Choose_J,
    Game_action_Choose_L,
    Game_action_Choose_S,
    Game_action_Choose_Z,
    Game_action_Choose_O,

    Game_action_Left = Game_state_Place << 5,
    Game_action_Right,
    Game_action_Rotate,
    Game_action_Drop,

    Game_action_Finish_clearing = Game_state_Cleared << 5
};

void init_piece_shape(Piece *);
void rotate_shape_cw(Tetrimino_shape);
void place_piece(const Piece *, Board, unsigned char);
int collides(const Piece *, const Board);
void drop_piece(Piece *, const Board);
void lift_piece(Piece *, const Board);
#endif /* ifndef XTETRIS_TETRIS_H */
