#include <stdio.h>
#include <unistd.h>
#include <common.h>

int
main(int argc, char **argv)
{
  struct lexer lex;
  char c;

  lex = lexer_init(HELLO_WORLD);
  while ((c = lexer_token(&lex)) != TOKEN_EOF) {
    lexer_print(c);
    printf(" (pos: %d)\n", lex.pos);
  }
  return 0;
}
