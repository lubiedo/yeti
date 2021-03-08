#include <stdio.h>
#include "common.h"

#define SKIP_SPACES(A)  while (*A == ' ' || *A == '\t') \
  A++

static struct parser *bfp;
static struct dbg_cmd cmds[] = {
  {DBG_QUIT, CMD_QUIT, "quit the debugger."},
  {DBG_INFO, CMD_INFO, "info of current session."},
  {DBG_PRNT, CMD_PRNT, "print memory region at pointer."\
    " `p (off)` for mem at offset."},
  {DBG_CONT, CMD_CONT, "continue until end of program."},
  {DBG_STEP, CMD_STEP, "perform a single step or `s (steps)` for N steps."},
  {DBG_BRKP, CMD_BRKP, "set breakpoint at curr position or `b (pos)`."},
  {DBG_HELP, CMD_HELP, "show help."},
  {-1, 0, NULL},
};

void
debugger_help()
{
  int i = 0;
  for (struct dbg_cmd c = cmds[i]; cmds[i++].id != -1; c = cmds[i])
    printf("%c:\t%s\n", c.cmd, c.help);
}

void
debugger_info(int brkps[])
{
  printf(
    "position: %d\t\tcurrent: %c\n"\
    " pointer: cell %u\t  value: %u\n"\
    " memsize: %d\n",
    bfp->lexer.pos, *(bfp->lexer.src - 1), // go back as src is pointing to next
    (unsigned int)(bfp->interpreter.ptr - bfp->interpreter.mem),
    *bfp->interpreter.ptr, MEMSIZE);

  if (brkps[0] > -1) {
    printf("breakpoints list:\n");
    for (int i = 0; brkps[i] > -1; i++)
      printf("\t[%d] %d\n", i, brkps[i]);
  }
}

extern int
debugger_isbrk(int p, int brkps[])
{
  int i = 0;

  while (brkps[i] > -1) {
    if (brkps[i] == p) {
      printf("b: hit breakpoint at pos %d\n", p);
      return 1;
    }
    i++;
  }
  return 0;
}

int
debugger_brkp(char *params, int brkps[])
{
  int pos = -1, retscan;

  if (*params == '\n') {
    pos = bfp->lexer.pos;
  } else {
    retscan = sscanf(params, "%d\n", &pos);
    if (!retscan || pos < 0) {
      printf("b: invalid position in code\n");
    }
  }

  if (pos > -1) {
    int i = 0;

    while (brkps[i] > -1) {
      if (i == MAX_BUFF) {
        printf("b: can't add more than %d breakpoints.\n", MAX_BUFF);
        return -1;
      }
      i++;
    }

    brkps[i] = pos;
    printf("breakpoint %d set at %d\n", i, brkps[i]);
    brkps[i + 1] = -1; // delim
  }

  return pos;
}

int
debugger_prnt(char *params)
{
  unsigned int offset = -1;
  int retscan;
  char *ptr = bfp->interpreter.mem;

  if (*params != '\n') {
    retscan = sscanf(params, "%d\n", &offset);
    if (!retscan || offset == -1) {
      printf("p: invalid offset\n");
      return offset;
    }
    if (offset > MEMSIZE) {
      printf("p: offset outside of memory\n");
      return offset;
    }
  } else {
    offset = bfp->interpreter.ptr - ptr;
  }

  ptr += offset;
  for (
      int i = 0, j = 0;
      offset <= MEMSIZE && j < PRNT_NVALUES;
      offset++, i++, j++
  ) {
    if (i == 16){ i = 0; putchar('\n'); }
    if (i == 0) printf("%d:", offset);
    if (i % 8 == 0) printf("\t");
    printf("%02x ", *ptr++);
  }
  putchar('\n');

  return 1;
}

int
debugger_step(char *params, int *out)
{
  unsigned int steps = -1;
  int retscan;

  if (*params != '\n') {
    retscan = sscanf(params, "%d\n", &steps);
    if (!retscan || steps == -1) {
      printf("s: invalid amount of steps\n");
      return steps;
    }
  } else {
    steps = 1;
  }
  if (steps > 1)
    printf("stepping %d times...\n", steps);
  *out = steps;

  return 1;
}

extern struct dbg_cmd
debugger_parser(struct parser *p, char *c, struct dbg_session *d)
{
  struct dbg_cmd ret = {-1, 0, NULL};
  char *ptr, cmd;
  int cmdid;

  bfp = p;

  SKIP_SPACES(c);

  cmd = *c;
  /* is the input empty? */
  if (cmd == '\n')
    return ret;

  ptr = c + 1;
  SKIP_SPACES(ptr);

  switch (cmd) { /* first char should be the command */
    case CMD_QUIT: cmdid = DBG_QUIT;break;
    case CMD_INFO: cmdid = DBG_INFO;break;
    case CMD_PRNT: cmdid = DBG_PRNT;break;
    case CMD_CONT: cmdid = DBG_CONT ;break;
    case CMD_STEP: cmdid = DBG_STEP ;break;
    case CMD_BRKP: cmdid = DBG_BRKP ;break;
    case CMD_HELP: cmdid = DBG_HELP ;break;
    default:
      printf("error: unkown debugger command\n");
      return ret;
      break;
  }

  /* commands with no params should have no extra chars except for spaces */
  switch (cmdid) {
    case DBG_QUIT: case DBG_INFO: case DBG_CONT: case DBG_HELP:
      if (*ptr != '\n') {
        printf("error: invalid usage of command `%c`\n", cmd);
        return ret;
      }
    break;
  }

  /* now process and carry away all commands */
  switch (cmdid) {
    case DBG_INFO:
      debugger_info(d->brkps);
      break;
    case DBG_PRNT:
      if (debugger_prnt(ptr) == -1)
        return ret;
      break;
    case DBG_STEP:
      if (debugger_step(ptr, &d->steps) == -1)
        return ret;
      break;
    case DBG_BRKP:
      if (debugger_brkp(ptr, d->brkps) == -1)
        return ret;
      break;
    case DBG_HELP:
      debugger_help();
      break;
  }

  ret = cmds[cmdid];

  return ret;
}
