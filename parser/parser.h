#pragma once
#include "../algebraic-c/algebraic.h"
#include <stdio.h>
printableType2Header(Cmd, 
    Note, char c, 
    Loop, int limit; 
          int seqno; 
          int looplen;
          struct CmdT * items, 
    LoopEnd,,
    SeqSep,,
    SeqEnd,,
    Speed,  int relative;
            int s, 
    Volume, int relative;
            int v, 
    Octave, int relative;
            int o, 
    Interp, char start;
            char end, 
    Save,, 
    Restore,, 
    End,, 
);

#define TYPE CmdT
#define _LIST_HEADER
#include "../list/list.h"
#undef _LIST_HEADER
#undef TYPE

// Stack to allow loops inside loops
struct list_sentinal_CmdT loop_stack;

/**
 * Clean up memory used by command
 * only necessary for Loop types
 *
 * @param e element to be destroyed
 */
void cmd_destroy(CmdT e);

// Falgs to track state inside loops
typedef enum {
  NONE = 0,
  NOLOOPS = 1,
} input_flags;

// History ring buffer type
typedef struct {
  CmdT items[100];
  int start, end;
} history_buffer;
// History ring buffer instance
history_buffer history;

/**
 * Get next input from parser. This function will not return a value of type
 * Loop and only Types that modify the state or output of the piano.
 *
 * @return CmdT next input for piano
 *
 * @param f tracks loop state pass in NONE when calling directly
 *
 */
CmdT getinput(input_flags f);

/**
 * Helper function to update loop stack and return updates from loop
 *
 * @return CmdT next input for piano
 *
 * @param f loop state
 * @param ret Loop type to start loop
 */
CmdT enterloop(input_flags f, CmdT ret);

/**
 * Initialize state of parser - call this before anything else
 */
void init_parser();

/**
 * Reset state of parser
 */
void reset_parser();

/**
 * Add element to history
 *
 * @param item element to be added to history
 */
void history_insert(CmdT item);

/**
 * Get most recent value from history
 */
CmdT history_pop();
