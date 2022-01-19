#ifndef XTETRIS_CONSTANTS_H
#define XTETRIS_CONSTANTS_H

#define BOARD_ROWS 15
#define BOARD_COLS 10
#define STARTING_PIECES 20

#define KEY_LEFT   'h'
#define KEY_RIGHT  'l'
#define KEY_DROP   'j'
#define KEY_ROTATE 'r'

#define KEY_I  'i'
#define KEY_T  't'
#define KEY_J  'j'
#define KEY_L  'l'
#define KEY_S  's'
#define KEY_Z  'z'
#define KEY_O  'o'

/* Points that get added to the score for clearing 1, 2, 3 or 4 lines respectively. */
static const int score_per_lines[] = {1, 3, 6, 12};

#endif /* ifndef XTETRIS_CONSTANTS_H */
