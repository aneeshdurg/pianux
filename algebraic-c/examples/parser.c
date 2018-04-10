#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "../algebraic.h"
#define LOG_INFO 0
#define LOG_WARN 1
#define LOG_ERR 2
#define LOG_NONE 3
typedef int log_t;
log_t should_log = LOG_NONE;
#define LOG(lvl, x) \
do{ \
  if(lvl>=should_log) x;\
}while(0)

NewAlgebraic2(
    Exp,
      Num,,
      Opr,,
      Opl,,
      Olr,,
      Prl,  struct ExpT *l;
            struct ExpT *r,
      Prr,  struct ExpT *l;
            struct ExpT *r,
      Prn,,
      Err,,
      Nop,,
);
static inline void exp_init(ExpT *e){
  memset(e, 0, sizeof(*e));
}

void printExp(ExpT a) {
  __PRINT_GEN(Num, a) 
  else __PRINT_GEN(Opr, a) 
  else __PRINT_GEN(Opl, a) 
  else __PRINT_GEN(Olr, a) 
  else __PRINT_GEN(Prl, a) 
  else __PRINT_GEN(Prr, a) 
  else __PRINT_GEN(Prn, a) 
  else __PRINT_GEN(Err, a) 
  else __PRINT_GEN(Nop, a) 
}

ExpT num;
ExpT prn;
ExpT opr;
ExpT opl;
ExpT olr;
ExpT err;
ExpT nop;

static inline int isInSet(char *set, char c){
  for(int i = 0; i < strlen(set); i++){
    if(c == set[i])
      return 1;
  }
  return 0;
}
static inline int isOp(char c){
  return isInSet("+-*/", c);
}

ExpT cast(char c){
  ExpT e;
  exp_init(&e);
  if(isdigit(c)){
    return num;
  } else if(isOp(c)){
    return olr;
  } else if(c == ')'){
    e.isPrr = 1;
    e.Prr.l = malloc(sizeof(ExpT));
    e.Prr.l->isNop = 1;
    e.Prr.r = malloc(sizeof(ExpT));
    e.Prr.r->isNop = 1;
  } else if(c == '('){
    e.isPrl = 1;
    e.Prl.l = malloc(sizeof(ExpT));
    e.Prl.l->isNop = 1;
    e.Prl.r = malloc(sizeof(ExpT));
    e.Prl.r->isNop = 1;
  } else{
    return err;
  }
  return e;
}

#define def_paren_op(s)                                                      \
ExpT paren_op_##s(ExpT a, ExpT b) {                                          \
  ExpT *val = malloc(sizeof(ExpT));                                          \
  ExpT operand = #s[0]=='l'?b:a;                                             \
  ExpT *branch = operand.Prr.s;                                                  \
  if(is_invalid_pr(a, b))                                                    \
    return err;                                                              \
  if(#s[0] == 'l')                                                           \
    *val = assocOp(a, *branch);                                              \
  else                                                                       \
    *val = assocOp(*branch, b);                                              \
  free(branch);                                                              \
  operand.Prr.s = val;                                                           \
  return operand;                                                            \
}

int is_invalid_pr(ExpT a, ExpT b){
  casefn(Opl, a, Prl, b, { return b.Prr.l->isNop; });
  casefn(Opr, a, Prr, b, { return b.Prr.l->isNop; });
  casefn(Num, a, Prl, b, { return b.Prr.l->isNop; });
  casefn(Prr, a, Opr, b, { return a.Prr.r->isNop; });
  casefn(Prr, a, Num, b, { return a.Prr.r->isNop; });
  casefn(Prl, a, Prr, b, { 
      return a.Prr.r->isNop && b.Prr.l->isNop; 
  });
  casefn(Prr, a, Prl, b, { 
      return a.Prr.r->isNop && b.Prr.l->isNop; 
  });
  return 0;
}

ExpT assocOp(ExpT a, ExpT b){
  def_paren_op(l); 
  def_paren_op(r); 

  casefn(Nop, a, _  , b, { return b; });
  casefn(_,   a, Nop, b, { return a; });
  casefn(Err, a, _  , b, { return a; });
  casefn(_,   a, Err, b, { return b; });

  casefn(Prn, a, Num, b, { return err; });
  casefn(Num, a, Prn, b, { return err; });
  casefn(Prn, a, _  , b, { a = num; });
  casefn(Prn, b, _  , a, { b = num; });

  casefn(Num, a, Num, b, { return num; });
  casefn(Num, a, Olr, b, { return opr; });
  casefn(Num, a, Opl, b, { return num; });
  casefn(Opr, a, Num, b, { return num; });
  casefn(Olr, a, Num, b, { return opl; });
 
  if(is_invalid_pr(a, b))
    return err;
  
  casefn(Prl, a, Prr, b, { 
      ExpT temp = assocOp(*a.Prl.r, b);
      $(Prr, temp, {
        temp = assocOp(*a.Prl.l, assocOp(*temp.Prr.l, *temp.Prr.r));
        $(Num, temp, return prn);
        return temp;
      });
      *a.Prl.r = nop;
      return assocOp(a, temp);
  });
  
  casefn(Prl, a, _  , b, { return paren_op_r(a, b); });
  casefn(Prr, a, _  , b, { return paren_op_r(a, b); });

  casefn(_  , a, Prl, b, { return paren_op_l(a, b); });
  casefn(_  , a, Prr, b, { return paren_op_l(a, b); });

  casefn(_  , a, _  , b, { return err; });
}

void strMap(char *input, ExpT *output, ExpT (*f)(char)){
  while(*input){
    *output = f(*input);
    input++;
    output++;
  }
}


int eval(char *str){
  ExpT *output = malloc(strlen(str)*sizeof(ExpT));
  strMap(str, output, cast);
  //fprintf(stderr, "%s\n", str);
  ExpT e = nop;
  for(int i = 0; i<strlen(str); i++){
    LOG(LOG_INFO, printf("%02d: ", i));
    LOG(LOG_INFO, printExp(e));
    LOG(LOG_INFO, printf("x "));
    LOG(LOG_INFO, printExp(output[i]));
    e = assocOp(e, output[i]);
    LOG(LOG_INFO, puts(""));
  }
  $(Prn, e, setType(Num, e));
  char *result = "INVALID";
  $(Num, e, result = "VALID");
  printf("Result: %s\n", result);
}

int main(int argc, char *argv[]){
  // Check argc count
  char* log_var = getenv("LOG");
  if(log_var)
    should_log = (log_t)atoi(log_var);
  setType(Num, num);
  setType(Prn, prn);
  setType(Opr, opr);
  setType(Opl, opl);
  setType(Olr, olr);
  setType(Err, err);
  setType(Nop, nop);

  return eval(argv[1]);
}
