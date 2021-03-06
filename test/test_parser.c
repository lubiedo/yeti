#include <stdio.h>
#include <unistd.h>
#include <common.h>

int
main(int argc, char **argv)
{
  struct parser parse;

  parse = parser_init(HELLO_WORLD, "/dev/stdout", 0, 0);
  parser_program(&parse);
  parser_fin(&parse);
  return 0;
}
