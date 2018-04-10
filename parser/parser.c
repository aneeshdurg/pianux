#include "parser.h"
#include "../algebraic-c/algebraic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Derive print functions for algebraic data type
mkprintfn(Cmd, 
    Note, 
    Speed,
    End
);

CmdT getinput(){
  CmdT ret;
  char c = getc(stdin);
  if( c == ' ' || (c >= 'a' && c <= 'g')) {
    setType(Note, ret);
    ret.Note.c = c;
  } else if (c >= '1' && c <= '9'){
    setType(Speed, ret);
    ret.Speed.s = c - '0';
  } else if (c == '+' || c == '-'){
    setType(Speed, ret);
    ret.Speed.relative = (( c == '+')?1:-1);
    c = getc(stdin);
    ret.Speed.s = c - '0';
  } else if(c == EOF || c == 'x'){
    setType(End, ret);
  } else {
    return getinput();
  }

  return ret;
}
