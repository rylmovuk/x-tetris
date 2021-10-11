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
 * Format used to show the number of pieces left when letting the player choose the next piece.
 * Sorry for all the string concatenaton jank -- wanted the source code to look aligned like the end result.
 */
static const char *piece_choice_prompt =
    " █ x%-2d  █▄ x%-2d  █▀ x%-2d  █  x%-2d  █▄ x%-2d  ▄█ x%-2d  ██ x%-2d\n"
    " █ <i> "" ▀  <t> "" ▀  <j> "" ▀▀ <l> ""  ▀ <s> "" ▀  <z> ""    <o>\n";

static const char *movement_prompt =
    "<h> move left    <l> move right\n"
    "    <r> rotate       <j> drop\n";
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
 * Format used to show the number of pieces left when letting the player choose the next piece.
 * Sorry for all the string concatenaton jank -- wanted the source code to look aligned like the end result.
 */
static const char *piece_choice_prompt =
    " I x%-2d   T x%-2d   J x%-2d   L x%-2d   S x%-2d   Z x%-2d   O x%-2d\n"
    "   <i> ""    <t> ""    <j> ""    <l> ""    <s> ""    <z> ""    <o>\n";

static const char *movement_prompt =
    "<h> move left    <l> move right\n"
    "    <r> rotate       <j> drop\n";
#endif
