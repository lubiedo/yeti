#include <stdio.h>
#include <unistd.h>

#include <common.h>

int
main(int argc, char **argv)
{
  struct parser parse;

  parse = parser_init(HELLO_WORLD, "test/helloworld.c");
  parser_program(&parse);
  parser_fin(&parse);

  parse = parser_init(CAT, "test/cat.c");
  parser_program(&parse);
  parser_fin(&parse);
  return 0;
}
