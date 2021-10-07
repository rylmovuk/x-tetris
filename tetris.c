#include <stdio.h>

#define BOARD_ROWS 15
#define BOARD_COLS 10

enum tetriminos {
    TETRIMINO_I = 1,
    TETRIMINO_T = 2,
    TETRIMINO_J = 3,
    TETRIMINO_L = 4,
    TETRIMINO_S = 5,
    TETRIMINO_Z = 6,
    TETRIMINO_O = 7
};


static char *    frame_top = "┏━━━━━━━━━━━━━━━━━━━━┓";
static char * frame_bottom = "┗━━━━━━━━━━━━━━━━━━━━┛";
static char * frame_border = "┃";

static const char * const block_types[] = {
    /*         (0) -> */ "  ",
    /* TETRIMINO_I -> */ "░░",
    /* TETRIMINO_T -> */ "▒▒",
    /* TETRIMINO_J -> */ "▓▓",
    /* TETRIMINO_L -> */ "██",
    /* TETRIMINO_S -> */ "░░",
    /* TETRIMINO_Z -> */ "▒▒",
    /* TETRIMINO_O -> */ "▓▓",
};

typedef struct Game {
    unsigned char board[BOARD_ROWS][BOARD_COLS];
} Game;


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

    draw(&game);

    return 0;
}
