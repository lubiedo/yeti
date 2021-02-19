#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "common.h"

#define DEFAULT_PERMS 0644

static void
clean(char *s, size_t l)
{
  while(l-- > 0)
    *(s+l) = '\0';
}

extern struct emitter
emitter_init(const char *path)
{
  struct emitter emit;

  emit.fd = open(path, O_RDWR | O_CREAT, DEFAULT_PERMS);
  if (emit.fd == -1) {
    perror("error");
    _abort(NULL, -1);
  }

  clean(emit.code, MAX_BUFF);
  return emit;
}

extern ssize_t
emitter_out(struct emitter *e)
{
  size_t  len   = strlen(e->code);
  ssize_t wrote = write(e->fd, e->code, len);

  clean(e->code, len);
  return wrote;
}

extern int
emitter_fin(struct emitter *e)
{
  return close(e->fd);
}
