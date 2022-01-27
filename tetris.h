/**
 * @file tetris.h
 * @author Maksim Kovalkov
 */

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
    /** The player must choose the next piece. */
    Game_state_Choose = 1,
    /** The player is moving the piece to the desired location. */
    Game_state_Place,

    Game_state_Lose,
    Game_state_Win,

    /** One or more lines have been cleared -- show it to the player before removing them */
    Game_state_Cleared
};

/** A possible kind of game: single-/multiplayer, 2 player or vs AI. */
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
    /** Playing fields for both players. */
    Board board[2];
    /** A piece that has been selected and is "about to be placed". */
    Piece active_piece;
    /** Scores for both players. */
    int score[2];
    /** A count for the amount of each piece left. */
    unsigned char pieces_left[7];
    /** Amount of lines cleared after last move. */
    int lines_cleared;
    /** Index of current player: 0 or 1. */
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

/**
 * Convenience function to set the shape of a piece during initialization, by reading the `type` field.
 */
void init_piece_shape(Piece *);
/**
 * Rotate a shape clockwise.
 */
void rotate_shape_cw(Tetrimino_shape);
/**
 * Place a piece on the board with no "collision" checking and assuming it fits inside the bounds,
 * setting each cell occupied by the piece to `type`.
 * Can be used to "cut out" a piece by filling with empty space or to draw "ghost pieces".
 * Note that x and y can actually be negative, and likewise y + 4 can be >= BOARD_WIDTH. It is the caller's
 * responsibility to ensure that no block compised by the piece ends up out of bounds -- in other words,
 * looking at piece->shape, **only zeroes** can end up outside of the board.
 */
void place_piece(const Piece *, Board, unsigned char);
/**
 * Check if the piece, when placed, would collide with existing blocks or with the outer bounds of the board.
 */
int collides(const Piece *, const Board);
/**
 * Drop the piece, in the conventional Tetris sense. More precisely, this means keeping its x value, and setting
 * its y value to put it as low as possible on the board without colliding with other pieces.
 */
void drop_piece(Piece *, const Board);
/**
 * Leave a piece's x value unchanged, and set its y value so that the piece is in the topmost position on the screen.
 */
void lift_piece(Piece *, const Board);
#endif /* ifndef XTETRIS_TETRIS_H */
