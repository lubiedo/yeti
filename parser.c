#include <stdio.h>
#include "common.h"

static struct dbg_cmd debug_cmd;
static struct dbg_session debug_sess = {{-1}, 0};

/* follow loop depth and loop instances */
static int loop_depth = 0;
static struct loop {
  int pos;
  struct loop *next;
} loops = {-1 ,NULL};
static struct loop *currentloop;

/* returns newly added loop */
static struct loop *
add_new_loop(struct loop *new)
{
  struct loop *l = loops.next;

  while (l->next != NULL)
    l = l->next;
  l->next = new;

  return l->next;
}

/* returns previous loop to the one removed */
static struct loop *
remove_last_loop()
{
  struct loop *l = loops.next, *prev;

  if (l->next == NULL)
    return l;

  while (l->next != NULL) {
    prev = l;
    l = l->next;
  }

#ifdef DEBUG
  printf("deleting loop at: %d\n", l->pos);
#endif
  free(l->next);
  l->next = NULL;
  prev->next = NULL;

  return prev;
}

#ifdef DEBUG
void
print_loops()
{
  struct loop *l = loops.next;
  int i = 1;

  while (l->next != NULL) {
    printf("loop #%d, pos: %d\n", i++, l->pos);
    l = l->next;
  }
  printf("loop #%d, pos: %d\n", i++, l->pos);
}
#endif

extern struct parser
parser_init(const char *s, const char *o, const int vm, const int dbg)
{
  struct parser parser;
  parser.lexer    = lexer_init(s);

  /* are we emitting any code or interpreting? */
  if (!vm) {
    parser.emitter  = emitter_init(o);
  } else {
    parser.interpreting++;
    parser.interpreter = interpreter_init(MEMSIZE);
  }

  /* should we enter into debugging mode */
  parser.debugging = dbg;

  return parser;
}

extern int
parser_fin(struct parser *p)
{
  if (p->interpreting)
    interpreter_fin(&p->interpreter);
  return p->interpreting ? EXIT_SUCCESS : emitter_fin(&p->emitter);
}

/* {statement} */
int
statement(struct parser *p)
{
  int current_token = lexer_token(&p->lexer), tmptoken, pos = p->lexer.pos;

#ifdef DEBUG
  printf("STATEMENT: ");
  lexer_print(current_token); printf(" (at pos: %d)\n", p->lexer.pos);
#endif

  /* if debugger mode is ON then ask for commands before anything
     also check if we are in a breakpoint pos */
  if (p->debugging || debugger_isbrk(pos, debug_sess.brkps)) {
    p->debugging = 1; // in case of brkp
    for (;;) {
      static char *debug_line;
      size_t debug_linecap = 0;

      /* are we doing multiple steps? */
      if (debug_sess.steps > 0 && --debug_sess.steps > 0)
        break;

      printf("yeti> "); // simple prompt
      getline(&debug_line, &debug_linecap, stdin);
      debug_cmd = debugger_parser(p, debug_line, &debug_sess);

      if (debug_cmd.id == -1) /* error or couldn't parse command */
        continue;

      /* immediately check if the command is to quit */
      if (debug_cmd.id == DBG_QUIT) {
        printf("quitting...\n");
        exit(EXIT_SUCCESS);
      }
      if (debug_cmd.id == DBG_CONT) {
        printf("continuing...\n");
        p->debugging = 0;
        break; /* free from loop and continue */
      }
      if (debug_cmd.id == DBG_STEP)
        break;
    }
  }

  /* not much as statements are simple in BF */
  switch (current_token) {
    case TOKEN_RIGHT:
      if (p->interpreting) interpreter_right(&p->interpreter);
        else scpy(p->emitter.code, "ptr++;\n"); break;
    case TOKEN_LEFT:
      if (p->interpreting) interpreter_left(&p->interpreter);
        else scpy(p->emitter.code, "ptr--;\n"); break;
    case TOKEN_INC:
      if (p->interpreting) interpreter_inc(&p->interpreter);
        else scpy(p->emitter.code, "(*ptr)++;\n"); break;
    case TOKEN_DEC:
      if (p->interpreting) interpreter_dec(&p->interpreter);
        else scpy(p->emitter.code, "(*ptr)--;\n"); break;
    case TOKEN_IN:
      if (p->interpreting) interpreter_in(&p->interpreter);
        else scpy(p->emitter.code, "*ptr = getchar();\n"); break;
    case TOKEN_OUT:
      if (p->interpreting) interpreter_out(&p->interpreter);
        else scpy(p->emitter.code, "putchar(*ptr);\n"); break;
    case TOKEN_OLOOP:
      loop_depth++;

      if (!p->interpreting) {
        scpy(p->emitter.code, "while (*ptr) {\n");
        emitter_out(&p->emitter);
      } else {
        /* save open loop position for future use */
        struct loop *newloop = (struct loop *) malloc(sizeof(struct loop));

        newloop->pos  = pos; // skip beginning of loop
        newloop->next = NULL;
        if (loops.next == NULL) {
          loops.next = newloop;
          currentloop = loops.next;
        } else {
          currentloop = add_new_loop(newloop);
        }
#ifdef DEBUG
        print_loops();
#endif
      }

      while ((tmptoken = statement(p)) != TOKEN_CLOOP) {
        if (tmptoken == TOKEN_EOF)
          _abort("open loop not closed", pos);
      }
      break;
    case TOKEN_CLOOP:
      if (!p->interpreting) {
        if (loop_depth) {
          loop_depth--;
        } else {
          _abort("closing unopen loop", pos);
        }

        p->emitter.code[0] = '}';
        p->emitter.code[1] = '\n';
      } else {
        /* check if loop must break, if not return to open loop position */
        if (*p->interpreter.ptr > (char)0) {
          p->lexer.src -= p->lexer.pos - currentloop->pos;
          p->lexer.pos = currentloop->pos;
        } else {
          currentloop = remove_last_loop();
        }
      }
      break;
  }

  if (!p->interpreting)
    emitter_out(&p->emitter);
  return current_token;
}

/* program ::= {statement} */
extern void
parser_program(struct parser *p)
{
#ifdef DEBUG
  printf("PROGRAM\n");
#endif
  if (!p->interpreting) {
    scpy(p->emitter.code, MAIN_INIT);
    emitter_out(&p->emitter);
  }

  if (p->debugging)
    printf("info: starting debugger, use `?` for help.\n");

  while (statement(p) != TOKEN_EOF) { ; }
  if (!p->interpreting) {
    scpy(p->emitter.code, MAIN_FIN);
    emitter_out(&p->emitter);
  }
}
