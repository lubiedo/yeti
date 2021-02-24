#include <stdio.h>
#include "common.h"

extern struct interpreter
interpreter_init(const int memlen)
{
  struct interpreter interpret;
  clean(interpret.mem, memlen);
  interpret.ptr = interpret.mem;

  return interpret;
}

extern void interpreter_right(struct interpreter *i){ i->ptr++; }
extern void interpreter_left(struct interpreter *i) { i->ptr--; }
extern void interpreter_inc(struct interpreter *i)  { (*i->ptr)++; }
extern void interpreter_dec(struct interpreter *i)  { (*i->ptr)--; }
extern void interpreter_in(struct interpreter *i)   { *i->ptr = gchar(); }
extern void interpreter_out(struct interpreter *i)  { pchar(*i->ptr); }
