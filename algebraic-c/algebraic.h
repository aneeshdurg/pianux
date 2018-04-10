#include "macro_iter.h"
#include <string.h>

#define newAlgebraic(type, ...)                                                \
  typedef struct type##T {                                                     \
    unsigned int is_ : 1;                                                      \
    __VA_ARGS__                                                                \
  } type##T;

#define NewTypeD(name, ...)                                                    \
  typedef struct name##_data { __VA_ARGS__; } name;

#define NewType(name) unsigned int is##name : 1;

#define NewType_(name, x) unsigned int is##name : 1;

#define DeclareData(type, ...)                                                 \
  union {                                                                      \
    __VA_ARGS__                                                                \
  };

#define DeclareData_(x) struct x##_data x;
#define DeclareData__(x, y) DeclareData_(x)

#define DeclareGetters(type, x)                                                \
  static inline struct x##_data *get##x(type##T *e) { return &(e->x); }
#define DeclareGetters_(type, x, y) DeclareGetters(type, x)

#define NewAlgebraic(type, ...)                                                \
  APPLY_ALL(NewTypeD, __VA_ARGS__);                                            \
  newAlgebraic(type, APPLY_ALL(NewType, __VA_ARGS__) DeclareData(              \
                         type, APPLY_ALL(DeclareData_, __VA_ARGS__)));         \
  P1_APPLY_ALL(DeclareGetters, type, __VA_ARGS__);

#define NewAlgebraic2(type, ...)                                               \
  APPLY_ALL_2(NewTypeD, __VA_ARGS__);                                          \
  newAlgebraic(type, APPLY_ALL_2(NewType_, __VA_ARGS__) DeclareData(           \
                         type, APPLY_ALL_2(DeclareData__, __VA_ARGS__)));      \
  P1_APPLY_ALL_2(DeclareGetters_, type, __VA_ARGS__);

#define __PRINT_GEN(type, val)                                                 \
  if (val.is##type)                                                            \
    printf("%s ", #type);

#define _printstmt(x) else __PRINT_GEN(x, a)
#define _printstmt_(x, y) _printstmt(x)
#define mkprintfn(type, ...)                                                   \
  void print##type##T(type##T a) {                                             \
    if (0) {                                                                   \
    }                                                                          \
    APPLY_ALL(_printstmt, __VA_ARGS__)                                         \
  }
#define mkprintfn2(type, ...)                                                  \
  void print##type##T(type##T a) {                                             \
    if (0) {                                                                   \
    }                                                                          \
    APPLY_ALL_2(_printstmt_, __VA_ARGS__)                                      \
  }

#define mkprintfnHeader(type) void print##type##T(type##T a);

#define printableType2(type, ...)                                              \
  NewAlgebraic2(type, __VA_ARGS__);                                            \
  mkprintfn2(type, __VA_ARGS__)

#define printableType2Header(type, ...)                                        \
  NewAlgebraic2(type, __VA_ARGS__);                                            \
  mkprintfnHeader(type)

#define printableType(type, ...)                                               \
  NewAlgebraic(type, __VA_ARGS__);                                             \
  mkprintfn(type, __VA_ARGS__)

#define printableTypeHeader(type, ...)                                         \
  NewAlgebraic(type, __VA_ARGS__);                                             \
  mkprintfnHeader(type, __VA_ARGS__)

#define setType(type, name)                                                    \
  do {                                                                         \
    memset(&name, 0, sizeof(name));                                            \
    name.is##type = 1;                                                         \
  } while (0)

#define $(typea, a, ...)                                                       \
  if ('_' == #typea[0]) {                                                      \
    __VA_ARGS__;                                                               \
  } else {                                                                     \
    if (a.is##typea) {                                                         \
      __VA_ARGS__;                                                             \
    }                                                                          \
  }

#define casefn(typea, a, typeb, b, ...) $(typea, a, $(typeb, b, __VA_ARGS__));
