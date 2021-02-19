#include <stdio.h>
#include "common.h"

static char next(struct lexer *l) { l->pos++; return *(l->src++); }

extern struct lexer
lexer_init(const char *s)
{
  struct lexer lex;
  lex.src = s;
  lex.pos = -1;
  return lex;
}

extern int
lexer_token(struct lexer *l)
{
  char c; /* holds current character */
  int token;

  c = next(l);
  switch (c) {
    case CHAR_RIGHT: token = TOKEN_RIGHT; break;
    case CHAR_LEFT: token = TOKEN_LEFT; break;
    case CHAR_INC: token = TOKEN_INC; break;
    case CHAR_DEC: token = TOKEN_DEC; break;
    case CHAR_OUT: token = TOKEN_OUT; break;
    case CHAR_IN: token = TOKEN_IN; break;
    case CHAR_OLOOP: token = TOKEN_OLOOP; break;
    case CHAR_CLOOP: token = TOKEN_CLOOP; break;
    case CHAR_EOF: token = TOKEN_EOF; break;
    default: token = TOKEN_UNKNOWN; /* ignore character */
  }
  return token;
}

extern void
lexer_print(int token)
{
  switch (token) {
    case TOKEN_RIGHT: printf("TOKEN_RIGHT"); break;
    case TOKEN_LEFT: printf("TOKEN_LEFT"); break;
    case TOKEN_INC: printf("TOKEN_INC"); break;
    case TOKEN_DEC: printf("TOKEN_DEC"); break;
    case TOKEN_OUT: printf("TOKEN_OUT"); break;
    case TOKEN_IN: printf("TOKEN_IN"); break;
    case TOKEN_OLOOP: printf("TOKEN_OLOOP"); break;
    case TOKEN_CLOOP: printf("TOKEN_CLOOP"); break;
    case TOKEN_EOF: printf("TOKEN_EOF"); break;
    default: printf("TOKEN_UNKNOWN"); break;
  }
}
