#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "common.h"

static char tmpdefault[17] = "/tmp/yetiXXXXXX";

static int verbose    = 0;
static int from_stdin = 0;
static int only_emit  = 0;
static int only_interpret = 0;

static void
help()
{
  fprintf(stderr, "USAGE:\tyeti [OPTIONS] [FILE|-]\n\n" \
    "OPTIONS:\n" \
    "\t-\tRead from stdin.\n"
    "\t-o\tOutput file path.\n" \
    "\t-e\tOnly emit C code.\n" \
    "\t-i\tInterpret bf code.\n" \
    "\t-v\tVerbose output.\n");
  exit(1);
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
  char *code;
  int inputfd;
  int opt;

  while ((opt = getopt(argc, argv, "hievo:")) != -1) {
    switch (opt) {
      case 'i':
        only_interpret++; break;
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
    if (*(*argv) == '-' && *(*argv + 1) == 0) {
      /* if - is file then read from stdin */
      from_stdin++;
    } else {
      input = *argv;
    }
  } else {
    fprintf(stderr, "error: missing FILE input\n");
    help();
  }

  if (only_emit && only_interpret)
    _abort("interpret and emit cannot be used at the same time", -1);

  if (output == NULL) {
    if (only_emit) {
      output = "a.c";
    } else if (only_interpret) {
      output = "-";
    } else {
      output = "a.out";
    }
  }

  if (verbose)
    printf("info: input file '%s', output file '%s'\n", input, output);

  /* consume input file code */
  size_t pgsize = getpagesize();
  if (!from_stdin) {
    if ((inputfd = open(input, O_RDONLY)) == -1 || stat(input, &inputstat) == -1)
    {
      perror("error");
      _abort(NULL, -1);
    }
    code = mmap(0, inputstat.st_size, PROT_READ, MAP_FILE|MAP_SHARED,
      inputfd, 0);
  } else {
    code = mmap(0, pgsize, PROT_READ|PROT_WRITE,
      MAP_ANONYMOUS|MAP_SHARED, -1, 0);
    read(STDIN_FILENO, code, pgsize);
  }

  /* child process will compile or emit the C code */
  int pipes[2];
  pid_t child;

  pipe(pipes);

  /* setup stdin/stdout for getchar() */
  close(pipes[0]);
  close(pipes[1]);
  dup2(STDOUT_FILENO, pipes[0]);
  dup2(STDIN_FILENO, pipes[1]);

  child = fork();
  if (child == 0) {
    dup2(pipes[0], STDIN_FILENO);
    dup2(pipes[1], STDOUT_FILENO);

    if (only_interpret) {
      /* interpret BF code */
      if (verbose)
        printf("info: interpret bf code.\n");
      parse = parser_init(code, NULL, 1);
    } else if (only_emit) {
      /* only emit the C code */
      if (verbose)
        printf("info: emit C code.\n");
      parse = parser_init(code, output, 0);
    } else {
      /* get temp file for generated C code */
      if (verbose)
        printf("info: compile code.\n");
      tmp = mktemp(tmpdefault);
      scat(tmp, ".c");

      if (verbose)
        printf("info: created temp file '%s'\n", tmp);

      parse = parser_init(code, tmp, 0);
    }

    parser_program(&parse);
    parser_fin(&parse);

    if (!only_emit && !only_interpret) {
      if (verbose)
        printf("info: compiling input file\n");
      compile(tmp, output);
    }

    exit(EXIT_SUCCESS);
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
      fprintf(stderr, "error: parser had an error\n");
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
