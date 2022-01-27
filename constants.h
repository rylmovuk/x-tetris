/**
 * @file constants.h
 * @author Maksim Kovalkov
 */

#ifndef XTETRIS_CONSTANTS_H
#define XTETRIS_CONSTANTS_H

/**
 * Constants that are useful for many of the files.
 */

#define BOARD_ROWS 15
#define BOARD_COLS 10
#define STARTING_PIECES 20

/**
 * Points that get added to the score for clearing 1, 2, 3 or 4 lines respectively.
 */
static const int score_per_lines[] = {1, 3, 6, 12};

#endif /* ifndef XTETRIS_CONSTANTS_H */
