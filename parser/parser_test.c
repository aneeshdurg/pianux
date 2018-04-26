#include "parser.h"
#include "../data_structures/algebraic-c/algebraic.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#define TEST(fn)                                                               \
  fn();                                                                        \
  puts(#fn " passed!");

void init_parser_test();
void history_insert_test();
void history_pop_test();
void history_realistic_test();
void getinput_simple_test();
void getinput_complex_test();
void getinput_loop_test();

int main() {
  TEST(init_parser_test);
  TEST(history_insert_test);
  TEST(history_pop_test);
  TEST(history_realistic_test); // TODO IMPLEMENT THIS

  TEST(getinput_simple_test);
  TEST(getinput_complex_test);
  TEST(getinput_loop_test);
}

void init_parser_test() {
  init_parser();
  assert(history.start == 0);
  assert(history.end == 0);
}

void history_insert_test() {
  CmdT new;
  setType(Note, new);
  new.Note.c = 'a';
  for (int i = 0; i < 100; i++) {
    history_insert(new);
    if (i < 99) {
      assert(history.start == 0);
      assert(history.end == i + 1);
    } else {
      assert(history.start == 1);
      assert(history.end == 0);
    }
  }
}

void history_pop_test() {
  CmdT item;
  for (int i = 0; i < 100; i++) {
    item = history_pop();
    assert(item.isNote);
    if (i < 99) {
      assert(item.Note.c == 'a');
      assert(history.start == (i + 2) % 100);
    } else {
      assert(item.Note.c == ' ');
      assert(history.start == (i + 2) % 100 - 1);
      assert(history.start == history.end);
    }
  }
}
void history_realistic_test() {
  // TODO
  assert(1);
}

static void populate_stdin(char *data, size_t len) {
  int p_input[2];

  pipe(p_input);
  dup2(p_input[0], 0);
  close(p_input[0]);

  write(p_input[1], data, len);
  close(p_input[1]);
}

void getinput_simple_test() {
  populate_stdin("c", 1);

  CmdT output = getinput(NONE);
  assert(output.isNote && output.Note.c == 'c');
}

static int cmpcmd(CmdT a, CmdT b) {
  casefn(Note, a, Note, b, { return a.Note.c == b.Note.c; });

  casefn(Speed, a, Speed, b, {
    return a.Speed.s == b.Speed.s && a.Speed.relative == b.Speed.relative;
  });

  casefn(Volume, a, Volume, b, {
    return a.Volume.v == b.Volume.v && a.Volume.relative == b.Volume.relative;
  });

  casefn(Octave, a, Octave, b, {
    return a.Octave.o == b.Octave.o && a.Octave.relative == b.Octave.relative;
  });

  casefn(Interp, a, Interp, b, {
    return a.Interp.start == b.Interp.start && a.Interp.end == b.Interp.end;
  });

  casefn(Loop, a, Loop, b, {
    if (a.Loop.limit == b.Loop.limit && a.Loop.looplen == b.Loop.looplen) {
      for (int i = 0; i < a.Loop.looplen; i++) {
        if (!cmpcmd(a.Loop.items[i], b.Loop.items[i]))
          return 0;
      }
      return 1;
    } else
      return 0;

  });

  casefn(Save, a, Save, b, { return 1; });

  casefn(Restore, a, Restore, b, { return 1; });

  casefn(End, a, End, b, { return 1; });

  return 0;
}
void getinput_complex_test() {
  char *input = "9c+1d-1c|a\"a~>b\n #1#-1e";
  populate_stdin(input, strlen(input));

  CmdT expected[13];
  setType(Speed, expected[0]);
  expected[0].Speed.s = 9;
  setType(Note, expected[1]);
  expected[1].Note.c = 'c';
  setType(Speed, expected[2]);
  expected[2].Speed.s = 1;
  expected[2].Speed.relative = 1;
  setType(Note, expected[3]);
  expected[3].Note.c = 'd';
  setType(Speed, expected[4]);
  expected[4].Speed.s = 1;
  expected[4].Speed.relative = -1;
  setType(Note, expected[5]);
  expected[5].Note.c = 'c';
  setType(Note, expected[6]);
  expected[6].Note.c = 'a';
  setType(Interp, expected[7]);
  expected[7].Interp.start = 'a';
  expected[7].Interp.end = 'b';
  setType(Note, expected[8]);
  expected[8].Note.c = ' ';
  setType(Octave, expected[9]);
  expected[9].Octave.o = 1;
  setType(Octave, expected[10]);
  expected[10].Octave.o = 1;
  expected[10].Octave.relative = -1;
  setType(Note, expected[11]);
  expected[11].Note.c = 'e';
  setType(End, expected[12]);

  for (int i = 0; i < 13; i++) {
    CmdT output = getinput(NONE);
    assert(cmpcmd(output, expected[i]));
  }
}
void getinput_loop_test() {
  char *input = "{abc,3}{{a,2}b,2}[def]";
  populate_stdin(input, strlen(input));

  CmdT loop1[5];
  setType(Save, loop1[0]);
  for (int i = 0; i < 3; i++) {
    setType(Note, loop1[i + 1]);
    loop1[i + 1].Note.c = 'a' + i;
  }
  setType(Restore, loop1[4]);

  // Second loop should be equivalent to {aab,2} + some save/restores
  CmdT loop2[9];
  setType(Save, loop2[0]);
  setType(Save, loop2[1]);
  setType(Note, loop2[2]);
  loop2[2].Note.c = 'a';
  setType(Restore, loop2[3]);
  setType(Save, loop2[4]);
  setType(Note, loop2[5]);
  loop2[5].Note.c = 'a';
  setType(Restore, loop2[6]);
  setType(Note, loop2[7]);
  loop2[7].Note.c = 'b';
  setType(Restore, loop2[8]);

  CmdT loop_final[5];
  setType(Save, loop_final[0]);
  for (int i = 0; i < 3; i++) {
    setType(Note, loop_final[i + 1]);
    loop_final[i + 1].Note.c = 'd' + i;
  }
  setType(Restore, loop_final[4]);

  CmdT output;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 5; j++) {
      output = getinput(NONE);
      assert(cmpcmd(loop1[j], output));
    }
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 9; j++) {
      output = getinput(NONE);
      assert(cmpcmd(loop2[j], output));
    }
  }

  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < 5; j++) {
      output = getinput(NONE);
      assert(cmpcmd(loop_final[j], output));
    }
  }
}
