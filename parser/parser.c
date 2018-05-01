#include "parser.h"
#include "../data_structures/algebraic-c/algebraic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Derive print function
mkprintfn(Cmd, 
    Note, 
    Loop,
    LoopEnd,
    SeqSep,
    SeqEnd,
    Speed,
    Volume,
    Octave,
    Interp,
    Save,
    Restore,
    End
);

/**
 * Parse for a loop body
 *
 * @param loop container to store loop in
 * @param delim delimer to stop parsing loop
 */
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

// CmdT list definition
#define TYPE CmdT
#define _LIST_IMPLEMENTATION
#include "../data_structures/list/list.h"
#undef _LIST_IMPLEMENTATION
#undef TYPE

/**
 * Destructor for CmdT
 *
 * @param e element to destroy
 */
void cmd_destroy(CmdT e) { $(Loop, e, free(e.Loop.items)); }

/**
 * Start getting input from inside loop
 *
 * @return CmdT next input for piano
 *
 * @param f flags from getinput
 * @param ret current loop
 */
CmdT enterloop(input_flags f, CmdT ret) {
  LIST_PREPEND(&loop_stack, ret);
  return getinput(f);
}

/**
 * Get input from loop
 *
 * @return CmdT next input for piano
 *
 * @param f flags from getinput
 */
static CmdT getloopinput(input_flags f) {
  Loop *l = getLoop(&(loop_stack.head->entry));

  if (l->limit) {
    CmdT ret;
    if (l->seqno == 0)
      setType(Save, ret);
    else if (l->seqno == l->looplen - 1)
      setType(Restore, ret);
    else
      ret = l->items[l->seqno - 1];

    l->seqno++;
    l->seqno %= l->looplen;
    if (l->seqno == 0 && l->limit > 0)
      l->limit--;
    $(Loop, ret, return enterloop(f, ret));
    history_insert(ret);
    return ret;
  }

  // Loop is done, push to history
  history_insert(loop_stack.head->entry);
  LIST_POPF(&loop_stack);
  return getinput(f);
}

/**
 * get next input for piano
 *
 * @return CmdT next input for piano
 *
 * @param f variable used to keep track of state
 */
CmdT getinput(input_flags f) {
  if (loop_stack.length)
    return getloopinput(f);

  CmdT ret;
  char c = getc(stdin);
  if (c == ' ' || (c >= 'a' && c <= 'g')) {
    setType(Note, ret);
    ret.Note.c = c;
  } else if (c >= '1' && c <= '9') {
    setType(Speed, ret);
    ret.Speed.s = c - '0';
  } else if (c == '+' || c == '-') {
    setType(Speed, ret);
    ret.Speed.relative = ((c == '+') ? 1 : -1);
    c = getc(stdin);
    ret.Speed.s = c - '0';
  } else if (c == 'v') {
    setType(Volume, ret);
    switch (getc(stdin)) {
    case '+':
      ret.Volume.relative = 1;
      break;
    case '-':
      ret.Volume.relative = -1;
      break;
    }
    ret.Volume.v = (getc(stdin) - '0');
  } else if (c == '#') {
    setType(Octave, ret);
    c = getc(stdin);
    switch (c) {
    case '+':
      ret.Octave.relative = 1;
      break;
    case '-':
      ret.Octave.relative = -1;
      break;
    }
    if (ret.Octave.relative)
      c = getc(stdin);
    ret.Octave.o = c - '0';
  } else if (c == '"') {
    char start = getc(stdin);
    char mid[2];
    mid[0] = getc(stdin);
    mid[1] = getc(stdin);
    char end = getc(stdin);
    if (!strncmp(mid, "~>", 2)) {
      setType(Interp, ret);
      ret.Interp.start = start;
      ret.Interp.end = end;
    } else
      return getinput(f);
  } else if (c == '.') {
    // Repeat last command
    ret = history_pop();
  } else if (c == 's') {
    setType(Save, ret);
  } else if (c == 'r') {
    setType(Restore, ret);
  } else if (c == '[') {
    // printf("Parsing loop\n");
    if (f == NOLOOPS)
      return getinput(f);
    setType(Loop, ret);
    ParseLoopBody(ret.Loop, LoopEnd);
    ret.Loop.limit = -1;
    // printf("Parsed loop\n");
  } else if (c == ']') {
    setType(LoopEnd, ret);
  } else if (c == '{') {
    // printf("Parsing seq\n");
    setType(Loop, ret);
    ParseLoopBody(ret.Loop, SeqSep);
    while (1) {
      char n = getc(stdin);
      // printf("    c: %c\n", n);
      if (n == '}')
        break;
      ret.Loop.limit *= 10;
      ret.Loop.limit += n - '0';
    }
    // printf("  Seq limit: %d\n", ret.Loop.limit);

    ret.Loop.seqno = 0;
    // printf("Parsed seq\n");
  } else if (c == ',') {
    setType(SeqSep, ret);
  } else if (c == EOF || c == 'x') {
    setType(End, ret);
  } else {
    return getinput(f);
  }

  $(Loop, ret, return enterloop(f, ret));
  history_insert(ret);
  return ret;
}

/**
 * Returns true iff the input is a type that should only be used internally
 * for the parser
 *
 * @return int 1 if meta type, 0 otherwise
 *
 * @param item instance that should be checked
 */
int meta_type(CmdT item) {
  $(LoopEnd, item, return 1);
  $(SeqSep, item, return 1);
  $(SeqEnd, item, return 1);
  $(Save, item, return 1);
  $(Restore, item, return 1);
  $(End, item, return 1);
  return 0;
}

void history_insert(CmdT item) {
  if (meta_type(item))
    return;
  history.items[history.end] = item;
  history.end += 1;
  history.end %= 100;
  if (history.end == history.start) {
    cmd_destroy(history.items[history.start]);
    history.start += 1;
    history.start %= 100;
  }
}

CmdT history_pop() {
  CmdT ret;
  if (history.start == history.end) {
    // If no availible history, return empty note
    setType(Note, ret);
    ret.Note.c = ' ';
    return ret;
  } else {
    ret = history.items[history.start];
    history.start += 1;
    history.start %= 100;
  }
  return ret;
}

void init_parser() {
  loop_stack = new_list(CmdT, cmd_destroy);
  history.start = 0;
  history.end = 0;
}

void reset_parser() { LIST_DESTROY(&loop_stack); }
