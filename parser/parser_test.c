#include "parser.h"
#include "../algebraic-c/algebraic.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
void getinput_simple_test();

int main() {
  getinput_simple_test();
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

  CmdT output = getinput();
  assert(output.isNote && output.Note.c == 'c');
}
