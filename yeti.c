#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "common.h"

static char tmpdefault[17] = "/tmp/yetiXXXXXX";

static int verbose    = 0;
static int only_emit = 0;

static void
help()
{
  fprintf(stderr, "USAGE:\tyeti [OPTIONS] [FILE|-]\n\n" \
    "OPTIONS:\n" \
    "\t-\tRead from stdin.\n"
    "\t-o\tOutput file path.\n" \
    "\t-e\tOnly emit C code.\n" \
    "\t-v\tVerbose output.\n");
  exit(1);
}

static char *
dirname(const char *path)
{
  char *p = strrchr(path, '/');

  if (p == NULL) {
    p = ".";
    return (p);
  }

  int dirlen = p - path;

  p = (char *) malloc(dirlen + 1);
  memcpy(p, path, dirlen);
  *(p + dirlen + 1) = '\0';

  return (p);
}

static int
compile(char *s, char *o)
{
  char  *cpath  = "/usr/bin/gcc";
  char *cargs[] = {
    cpath, s, "-O3", "-Wall", "-std=c99", "-o", o, NULL
  };
  return execv(cpath, cargs);
}

int
main(int argc, char **argv)
{
  struct parser parse;
  struct stat inputstat;
  char *tmp = NULL, *output = NULL, *input;
  char *code, *filedir;
  int inputfd;
  int opt;

  while ((opt = getopt(argc, argv, "hevo:")) != -1) {
    switch (opt) {
      case 'e':
        only_emit++; break;
      case 'v':
        verbose++; break;
      case 'o':
        output = optarg; break;
      case 'h': default:
        help();
    }
  }
  argc -= optind;
  argv += optind;

  if (argc == 1) {
    if (*(*argv) == '-' && *(*argv + 1) == 0)
      /* if - is file then read from stdin */
      input = "/dev/stdin";
    else
      input = *argv;
  } else {
    fprintf(stderr, "error: missing FILE input\n");
    help();
  }

  if (output == NULL) {
    if (only_emit) {
      output = "a.c";
    } else {
      output = "a.out";
    }
  }

  if (verbose)
    printf("info: input file '%s', output file '%s'\n", input, output);

  /* consume input file code */
  if ((inputfd = open(input, O_RDONLY)) == -1 || stat(input, &inputstat) == -1)
  {
    perror("error");
    _abort(NULL, -1);
  }
  code = mmap(0, inputstat.st_size, PROT_READ, MAP_FILE|MAP_SHARED, inputfd, 0);
  filedir   = dirname(input);

  /* child process will compile or emit the C code */
  pid_t child = fork();
  if (child == 0) {
    if (verbose)
      printf("info: changing directory to '%s'.\n", filedir);
    chdir(filedir);

    if (only_emit) {
      /* only emit the C code */
      parse = parser_init(code, output);
    } else {
      /* get temp file for generated C code */
      tmp = mktemp(tmpdefault);
      strcat(tmp, ".c");
      if (verbose)
        printf("info: created temp file '%s'\n", tmp);

      parse = parser_init(code, tmp);
    }

    parser_program(&parse);
    parser_fin(&parse);

    if (verbose)
      printf("info: compiling input file\n");
    compile(tmp, output);
  }

  /* check if child failed and wait for it to be done */
  if (child == -1) {
    perror("fork");
  } else {
    int childstat;
    if (waitpid(child, &childstat, 0) == -1) {
      perror("wait");
    }

    if (WIFEXITED(childstat) ? WEXITSTATUS(childstat) : EXIT_FAILURE) {
      fprintf(stderr, "error: compiler had an error\n");
    }
  }

  /* final */
  if (verbose)
    printf("info: done!\n");
  if (tmp != NULL)
    unlink(tmp);
  close(inputfd);
  munmap(code, inputstat.st_size);
  return 0;
}
