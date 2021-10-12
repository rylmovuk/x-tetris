#ifdef UTF8_SUPPORT
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
    /* BLOCK_GHOST -> */ "⣏⣹",
    /* BLOCK_CLEAR -> */ "⣶⣶",
    /* BLOCK_BADBK -> */ "‼︎‼︎"
};

/**
 * All the information that gets displayed on the side of the main board.
 * Sorry for all the string concatenaton jank -- wanted the source code to look aligned
 * "close enough" to the end result.
 */
static const char *interface[] = {
    "   =*= X - T E T R I S =*=   ",
    "                             ",
    "        score:   %3d         ",
    "                             ",
    "╭───────────────────────────╮",
    "│ "        "%25s"         " │",
    "│ "        "%25s"         " │",
    "╰──────────────────────────┬╯",
    "                           ╰ ",
    "                             ",
    "█ x%-2d  █▄ x%-2d   █ x%-2d  █  x%-2d",
    "█ <i> "" ▀  <t> "" ▀▀ <j> "" ▀▀ <l>",
    "                             ",
    "    █▄ x%-2d  ▄█ x%-2d  ██ x%-2d   ",
    "     ▀ <s> "" ▀  <z>  ""   <o>   ",
};

/* FIXME: get rid of at least some of the hardcoded values
   -- e.g. dont redefine score values here */
static const char *(messages[][2]) = {
    /* GAME_STATE_CHOOSE -> */
    {"  choose which tetrimino ",
     "    you want to place    "},
    /* GAME_STATE_PLACE -> */
    {"<h>, <l> move left, right",
     "  <r> rotate   <j> drop  "},
    /* GAME_STATE_LOSE -> */
    {"    oh no... you lost!   ",
     "can't place another piece"},
    /* GAME_STATE_WIN -> */
    {"congratulations! you won!",
     " check your final score  "},
    
    {0, 0},
    /* GAME_STATE_CLEARED + {1,2,3,4} -> */
    {"          line!          ",
     "   you earned 1 point    "},
    {"         double!!        ",
     "   you earned 3 points   "},
    {"        triple!!!        ",
     "   you earned 6 points   "},
    {"      !! TETRIS !!       ",
     "  you earned 12 points   "}
};
#else
/* String constants for drawing the frame. */
static char *    frame_top = "+--------------------+";
static char * frame_bottom = "+--------------------+";
static char * frame_border = "|";

/* Strings each block inside the game board gets translated to. */
static const char * const block_types[] = {
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

/**
 * All the information that gets displayed on the side of the main board.
 * Sorry for all the string concatenaton jank -- wanted the source code to look aligned
 * "close enough" to the end result.
 */
static const char *interface[] = {
    "   =*= X - T E T R I S =*=   ",
    "                             ",
    "        score:   %3d         ",
    "                             ",
    " ........................... ",
    "' "        "%25s"         " '",
    "' "        "%25s"         " '",
    "'...........................'",
    "                           \\ ",
    "                             ",
    "I x%-2d   T x%-2d   J x%-2d   L x%-2d",
    "  <i> ""    <t> ""    <j> ""    <l>",
    "                             ",
    "     S x%-2d   Z x%-2d   O x%-2d   ",
    "       <s> ""    <z>  ""   <o>   ",
};

/* FIXME: get rid of at least some of the hardcoded values
   -- e.g. dont redefine score values here */
static const char *(messages[][2]) = {
    /* GAME_STATE_CHOOSE -> */
    {"  choose which tetrimino ",
     "    you want to place    "},
    /* GAME_STATE_PLACE -> */
    {"<h>, <l> move left, right",
     "  <r> rotate   <j> drop  "},
    /* GAME_STATE_LOSE -> */
    {"    oh no... you lost!   ",
     "can't place another piece"},
    /* GAME_STATE_WIN -> */
    {"congratulations! you won!",
     " check your final score  "},
    
    /* GAME_STATE_CLEARED + {1,2,3,4} -> */
    {0, 0},
    {"          line!          ",
     "   you earned 1 point    "},
    {"         double!!        ",
     "   you earned 3 points   "},
    {"        triple!!!        ",
     "   you earned 6 points   "},
    {"      !! TETRIS !!       ",
     "   you earned 12 point   "}
};
#endif
