#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"

#include "iohandler.h"

#define SCREEN_LINES 17
#define SCREEN_COLUMNS 54
#define MSG_LENGTH 25
#define INPUT_BUF_LEN 32

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

#define CHARUPPER(c) ((c) - 'a' + 'A')

struct Io_handler {
    char **screen;
    char input_buf[INPUT_BUF_LEN];
    unsigned input_i;
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
    /* Block_type_Empty -> */ "  ",
    /* Tetrimino_type_I -> */ "@@",
    /* Tetrimino_type_T -> */ "##",
    /* Tetrimino_type_J -> */ "$$",
    /* Tetrimino_type_L -> */ "%%",
    /* Tetrimino_type_S -> */ "@@",
    /* Tetrimino_type_Z -> */ "##",
    /* Tetrimino_type_O -> */ "$$",
    /* Block_type_Ghost -> */ "()",
    /* Block_type_Clear -> */ "><",
    /* Block_type_Badbk -> */ "!!"
};

static char const cleared_lines_messages[][MSG_LENGTH] = {
    /* 1 -> */ "          line!          ",
    /* 2 -> */ "         double!!        ",
    /* 3 -> */ "        triple!!!        ",
    /* 4 -> */ "      !! TETRIS !!       "
};

static char const *prompts[] = {
    /* Game_state_Choose  -> */ "[itjlszo] > ",
    /* Game_state_Place   -> */ "[hlrj]+ > ",
    /* Game_state_Lose    -> */ "",
    /* Game_state_Win     -> */ "",
    /* Game_state_Cleared -> */ "[<enter>] > ",
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
    /* fld_playing_field  -> */ { 1,  1},
    /* fld_score          -> */ { 3, 41},
    /* fld_message_bubble -> */ { 6, 26},
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
iohandler_create()
{
    int i;

    Io_handler *ioh = malloc_or_die(sizeof(Io_handler));
    ioh->screen = alloc_many_or_die(SCREEN_LINES, SCREEN_COLUMNS * sizeof(char));
    for (i = 0; i < SCREEN_LINES; ++i) {
        memcpy(ioh->screen[i], screen_init_state[i], SCREEN_COLUMNS);
        ioh->screen[i][SCREEN_COLUMNS-1] = 0;
    }

    ioh->input_i = 0;
    ioh->input_buf[0] = 0;

    setvbuf(stdout, NULL, _IOFBF, 4096);
    return ioh;
}

void
iohandler_destroy(Io_handler *ioh)
{
    int i;
    for (i = 0; i < SCREEN_LINES; ++i)
        free(ioh->screen[i]);
    free(ioh->screen);
}

/** 
 * Update every field on the screen to reflect the game state. 
 */
void
update_screen_1p(char **scr, Game const *game)
{
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
        case Game_state_Choose:
            memcpy(&scr[line][col],   "  choose which tetrimino ", MSG_LENGTH);
            memcpy(&scr[line+1][col], "    you want to place    ", MSG_LENGTH);
            break;
        case Game_state_Place:
            sprintf(buf, "<%c>, <%c> move left, right", KEY_LEFT, KEY_RIGHT);
            memcpy(&scr[line][col], buf, MSG_LENGTH);
            sprintf(buf, "  <%c> rotate   <%c> drop  ", KEY_ROTATE, KEY_DROP);
            memcpy(&scr[line+1][col], buf, MSG_LENGTH);
            break;
        case Game_state_Lose:
            memcpy(&scr[line][col], "    oh no... you lost!   ", MSG_LENGTH);
            memcpy(&scr[line+1][col], "can't place another piece", MSG_LENGTH);
            break;
        case Game_state_Win:
            memcpy(&scr[line][col], "congratulations! you won!", MSG_LENGTH);
            memcpy(&scr[line+1][col], " check your final score  ", MSG_LENGTH);
            break;
        case Game_state_Cleared:
            memcpy(&scr[line][col], cleared_lines_messages[game->lines_cleared-1], MSG_LENGTH);
            sprintf(
                buf,
                "   you earned %2d %s  ",
                score_per_lines[game->lines_cleared-1],
                (score_per_lines[game->lines_cleared-1] == 1) ? "point " : "points"
            );
            memcpy(&scr[line+1][col], buf, MSG_LENGTH);
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
            if (game->state == Game_state_Choose) 
                sprintf(buf, "<%c>", key_hints[i]);
            else
                buf[0] = buf[1] = buf[2] = ' ';
            memcpy(&scr[line][col], buf, 3);
        }
    }

}

void
iohandler_draw_1p(Io_handler *ioh, Game const *game)
{
    /* Update the screen, display it, and print the prompt.  
       Due to the "asyncronous" nature of this style of interaction, we can immediately read and store the input
       to be processed later by the other function. */
    int i;

    update_screen_1p(ioh->screen, game);

    for (i = 0; i < SCREEN_LINES; ++i) {
        puts(ioh->screen[i]);
    }

    fputs(prompts[game->state-1], stdout);
    fflush(stdout);
    ioh->input_i = 0;
    /* keep reading until newline but at most INPUT_BUF_LEN chars; throw out whatever is left */
    for (i = 0; i < INPUT_BUF_LEN-1; ++i) {
        if ((ioh->input_buf[i] = getchar()) == '\n')
            break;
    }
    if (i == INPUT_BUF_LEN-1) {
        while (getchar() != '\n') /* discard */;
    }
    ioh->input_buf[i] = 0;
}

enum Game_action
iohandler_next_action_1p(Io_handler *ioh, Game const *game)
{
    char c;
    
    do {
        c = ioh->input_buf[ioh->input_i++];
    } while (c == ' '); /* no buf overflow: will reach '\0' eventually */ 

    switch (game->state) {
    case Game_state_Choose:
        switch (c) {
        case KEY_I: case CHARUPPER(KEY_I):
            return Game_action_Choose_I;
        case KEY_T: case CHARUPPER(KEY_T):
            return Game_action_Choose_T;
        case KEY_J: case CHARUPPER(KEY_J):
            return Game_action_Choose_J;
        case KEY_L: case CHARUPPER(KEY_L):
            return Game_action_Choose_L;
        case KEY_S: case CHARUPPER(KEY_S):
            return Game_action_Choose_S;
        case KEY_Z: case CHARUPPER(KEY_Z):
            return Game_action_Choose_Z;
        case KEY_O: case CHARUPPER(KEY_O):
            return Game_action_Choose_O;
        }
        break;
    case Game_state_Place:
        switch (c) {
        case   KEY_LEFT: case   CHARUPPER(KEY_LEFT):
            return Game_action_Left;
        case  KEY_RIGHT: case  CHARUPPER(KEY_RIGHT):
            return Game_action_Right;
        case KEY_ROTATE: case CHARUPPER(KEY_ROTATE):
            return Game_action_Rotate;
        case   KEY_DROP: case   CHARUPPER(KEY_DROP):
            return Game_action_Drop;
        }
        break;
    case Game_state_Cleared:
        return Game_action_Finish_clearing;
    case Game_state_Lose:
    case Game_state_Win:
        break;
    /* exhaustive */
    }

    return Game_action_Queue_empty;
}
