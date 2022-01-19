#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"

#include "iohandler.h"

#define SCREEN_LINES 17
#define SCREEN_COLUMNS 54

#define CHARUPPER(c) ((c) - 'a' + 'A')

struct Io_handler {
    char **screen;
    char *input_buf;
};

static void update_screen_1p(char **, Game const *);

static const char screen_init_state[SCREEN_LINES][SCREEN_COLUMNS] = {
    "+--------------------+                               ",
    "|                    |     =*= X - T E T R I S =*=   ",
    "|                    |                               ",
    "|                    |          score:   000         ",
    "|                    |                               ",
    "|                    |   ........................... ",
    "|                    |  '                           '",
    "|                    |  '                           '",
    "|                    |  '...........................'",
    "|                    |                             \\ ",
    "|                    |                               ",
    "|                    |  I x00   T x00   J x00   L x00",
    "|                    |    < >     < >     < >     < >",
    "|                    |                               ",
    "|                    |       S x00   Z x00   O x00   ",
    "|                    |         < >     < >     < >   ",
    "+--------------------+                               ",
};

static char const block_types[][2] = {
    /* BLOCK_EMPTY -> */ "  ",
    /* TETRIMINO_I -> */ "@@",
    /* TETRIMINO_T -> */ "##",
    /* TETRIMINO_J -> */ "$$",
    /* TETRIMINO_L -> */ "%%",
    /* TETRIMINO_S -> */ "@@",
    /* TETRIMINO_Z -> */ "##",
    /* TETRIMINO_O -> */ "$$",
    /* BLOCK_GHOST -> */ "()",
    /* BLOCK_CLEAR -> */ "><",
    /* BLOCK_BADBK -> */ "!!"
};

static char const cleared_lines_messages[][25] = {
    "          line!          ",
    "         double!!        ",
    "        triple!!!        ",
    "      !! TETRIS !!       "
};

enum Updatable_field {
    fld_playing_field,
    fld_score,
    fld_message_bubble,
    fld_count_i,
    fld_count_t,
    fld_count_j,
    fld_count_l,
    fld_count_s,
    fld_count_z,
    fld_count_o,
    fld_key_i,
    fld_key_t,
    fld_key_j,
    fld_key_l,
    fld_key_s,
    fld_key_z,
    fld_key_o
};

static int const field_coords[][2] = {
    /* fld_playing_field  -> */ {1, 1},
    /* fld_score          -> */ {3, 41},
    /* fld_message_bubble -> */ {6, 26},
    /* fld_count_i        -> */ {11, 27},
    /* fld_count_t        -> */ {11, 35},
    /* fld_count_j        -> */ {11, 43},
    /* fld_count_l        -> */ {11, 51},
    /* fld_count_s        -> */ {14, 32},
    /* fld_count_z        -> */ {14, 40},
    /* fld_count_o        -> */ {14, 48},
    /* fld_key_i          -> */ {12, 26},
    /* fld_key_t          -> */ {12, 34},
    /* fld_key_j          -> */ {12, 42},
    /* fld_key_l          -> */ {12, 50},
    /* fld_key_s          -> */ {15, 31},
    /* fld_key_z          -> */ {15, 39},
    /* fld_key_o          -> */ {15, 47},
};

static char const key_hints[7] = { KEY_I, KEY_T, KEY_J, KEY_L, KEY_S, KEY_Z, KEY_O };

Io_handler *
iohandler_create() {
    int i;

    Io_handler *ioh = malloc_or_die(sizeof(Io_handler));
    ioh->screen = alloc_many_or_die(SCREEN_LINES, SCREEN_COLUMNS * sizeof(char));
    for (i = 0; i < SCREEN_LINES; ++i) {
        memcpy(ioh->screen[i], screen_init_state[i], SCREEN_COLUMNS);
        ioh->screen[i][SCREEN_COLUMNS-1] = 0;
    }

    setvbuf(stdout, NULL, _IOFBF, 4096);
    return ioh;
}

void
iohandler_destroy(Io_handler *ioh) {
    int i;
    for (i = 0; i < SCREEN_LINES; ++i)
        free(ioh->screen[i]);
    free(ioh->screen);
}

void
update_screen_1p(char **scr, Game const *game) {
    int line, col, i;
    char buf[32];
    { /* update board */
        int y, x;
        line = field_coords[fld_playing_field][0];
        col = field_coords[fld_playing_field][1];
        for (y = 0; y < BOARD_ROWS; ++y) {
            for (x = 0; x < BOARD_COLS; ++x) {
                memcpy(
                    &scr[line + y][col + x*2],
                    block_types[game->board[y][x]],
                    2
                );
            }
        }
    }

    { /* update score */
        line = field_coords[fld_score][0];
        col = field_coords[fld_score][1];
        sprintf(buf, "%3d", game->score);
        memcpy(&scr[line][col], buf, 3);
    }

    { /* update message */ 
        line = field_coords[fld_message_bubble][0];
        col = field_coords[fld_message_bubble][1];
        switch (game->state) {
        case Game_state_choose:
            memcpy(&scr[line][col],   "  choose which tetrimino ", 25);
            memcpy(&scr[line+1][col], "    you want to place    ", 25);
            break;
        case Game_state_place:
            sprintf(buf, "<%c>, <%c> move left, right", KEY_LEFT, KEY_RIGHT);
            memcpy(&scr[line][col], buf, 25);
            sprintf(buf, "  <%c> rotate   <%c> drop  ", KEY_ROTATE, KEY_DROP);
            memcpy(&scr[line+1][col], buf, 25);
            break;
        case Game_state_lose:
            memcpy(&scr[line][col], "    oh no... you lost!   ", 25);
            memcpy(&scr[line+1][col], "can't place another piece", 25);
            break;
        case Game_state_win:
            memcpy(&scr[line][col], "congratulations! you won!", 25);
            memcpy(&scr[line+1][col], " check your final score  ", 25);
            break;
        case Game_state_cleared:
            memcpy(&scr[line][col], cleared_lines_messages[game->lines_cleared-1], 25);
            sprintf(
                buf,
                "   you earned %d %s   ",
                score_per_lines[game->lines_cleared-1],
                (score_per_lines[game->lines_cleared-1] == 1) ? "point " : "points"
            );
            memcpy(&scr[line+1][col], buf, 25);
            break;
        }
    }

    { /* update piece counts */
        for (i = 0; i <= 7; ++i) {
            line = field_coords[fld_count_i + i][0];
            col = field_coords[fld_count_i + i][1];
            sprintf(buf, "%-2hu", (unsigned short)game->pieces_left[i]);
            memcpy(&scr[line][col], buf, 2);
        }
    }

    { /* update key hints */
        for (i = 0; i < 7; ++i) {
            line = field_coords[fld_key_i + i][0];
            col = field_coords[fld_key_i + i][1];
            if (game->state == Game_state_choose) 
                sprintf(buf, "<%c>", key_hints[i]);
            else
                buf[0] = buf[1] = buf[2] = ' ';
            memcpy(&scr[line][col], buf, 3);
        }
    }

}

void
iohandler_draw_1p(Io_handler *ioh, Game *game) {
    int i;

    update_screen_1p(ioh->screen, game);

    for (i = 0; i < SCREEN_LINES; ++i) {
        puts(ioh->screen[i]);
    }

    fflush(stdout);
}
