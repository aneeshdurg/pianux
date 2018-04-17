#pragma once
#include "../algebraic-c/algebraic.h"
#include <stdio.h>
printableType2Header(Cmd, Note, char c, Loop, int limit; int seqno; int looplen;
                     struct CmdT * items, LoopEnd, , SeqSep, , SeqEnd, , Speed,
                     int relative;
                     int s, Volume, int relative; int v, Octave, int relative;
                     int o, Interp, char start;
                     char end, Save, , Restore, , End, , );

#define TYPE CmdT
#define _LIST_HEADER
#include "../list/list.h"
#undef _LIST_HEADER
#undef TYPE

struct list_sentinal_CmdT loop_stack;
void cmd_destroy(CmdT e);

typedef enum {
  NONE = 0,
  NOLOOPS = 1,
} input_flags;

typedef struct {
  CmdT items[100];
  int start, end;
} history_buffer;

history_buffer history;

CmdT getinput(input_flags f);
CmdT enterloop(input_flags f, CmdT ret);
void init_parser();
void reset_parser();
void history_insert(CmdT item);
CmdT history_pop();

#define ParseLoopBody(loop, delim)                                             \
  loop.items = malloc(256 * sizeof(CmdT));                                     \
  int i = 0;                                                                   \
  while (i < 256) {                                                            \
    CmdT next = getinput(NOLOOPS);                                             \
    if (next.is##delim) {                                                      \
      break;                                                                   \
    }                                                                          \
    loop.items[i] = next;                                                      \
    i++;                                                                       \
  }                                                                            \
  loop.looplen = i + 2;                                                        \
  loop.limit = 0;                                                              \
  loop.seqno = 0;
