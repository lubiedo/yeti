#include <stdio.h>
#include <string.h>
#include "common.h"

/* follow loop depth */
static int loop_depth = 0;

extern struct parser
parser_init(const char *s, const char *o)
{
  struct parser parser;
  parser.lexer    = lexer_init(s);
  parser.emitter  = emitter_init(o);
  return parser;
}

extern int
parser_fin(struct parser *p)
{
  return emitter_fin(&p->emitter);
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

  /* not much as statements are simple in BF */
  switch (current_token) {
    case TOKEN_RIGHT:
      strcpy(p->emitter.code, "ptr++;\n"); break;
    case TOKEN_LEFT:
      strcpy(p->emitter.code, "ptr--;\n"); break;
    case TOKEN_INC:
      strcpy(p->emitter.code, "(*ptr)++;\n"); break;
    case TOKEN_DEC:
      strcpy(p->emitter.code, "(*ptr)--;\n"); break;
    case TOKEN_OUT:
      strcpy(p->emitter.code, "putchar(*ptr);\n"); break;
    case TOKEN_IN:
      strcpy(p->emitter.code, "*ptr = getchar();\n"); break;
    case TOKEN_OLOOP:
      loop_depth++;

      strcpy(p->emitter.code, "while (*ptr) {\n");
      emitter_out(&p->emitter);

      while ((tmptoken = statement(p)) != TOKEN_CLOOP) {
        if (tmptoken == TOKEN_EOF)
          _abort("open loop not closed", pos);
      }
      break;
    case TOKEN_CLOOP:
      if (loop_depth)
        loop_depth--;
      else
        _abort("closing unopen loop", pos);

      p->emitter.code[0] = '}';
      p->emitter.code[1] = '\n';
      break;
  }

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
  strcpy(p->emitter.code, MAIN_INIT);
  emitter_out(&p->emitter);

  while (statement(p) != TOKEN_EOF) { ; }

  strcpy(p->emitter.code, MAIN_FIN);
  emitter_out(&p->emitter);
}
