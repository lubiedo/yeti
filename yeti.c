#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "common.h"

#define ONLY(A) if (action != A) \
  _abort("multiple actions cannot be used at the same time (-e, -d, -i)", -1);

/* available actions */
enum {
  COMPILE = 1,
  EMIT,
  INTERPRET,
  DEBUG
};

static char tmpdefault[17] = "/tmp/yetiXXXXXX";

static int verbose    = 0;
static int from_stdin = 0;
static int action     = COMPILE;

static void
help()
{
  fprintf(stderr, "USAGE:\tyeti [OPTIONS] [FILE|-]\n\n" \
    "OPTIONS:\n" \
    "\t-\tRead from stdin.\n"
    "\t-o\tOutput file path.\n" \
    "\t-d\tDebug mode.\n" \
    "\t-e\tOnly emit C code.\n" \
    "\t-i\tInterpret bf code.\n" \
    "\t-v\tVerbose output.\n");
  exit(1);
}

static int
compile(char *s, char *o)
{
  char *cpath   = "/usr/bin/gcc";
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

  while ((opt = getopt(argc, argv, "hidevo:")) != -1) {
    switch (opt) {
      case 'i':
        ONLY(COMPILE)
        action = INTERPRET; break;
      case 'd':
        ONLY(COMPILE)
        action = DEBUG; break;
      case 'e':
        ONLY(COMPILE)
        action = EMIT; break;
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

  if (output == NULL) {
    if (action == EMIT) {
      output = "a.c";
    } else if (action == INTERPRET || action == DEBUG) {
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

    if (action == INTERPRET || action == DEBUG) {
      /* interpret BF code */
      if (verbose)
        printf("info: %s bf code.\n",
          (action == INTERPRET ? "interpret" : "debug"));

      parse = parser_init(code, NULL, 1, action - INTERPRET);
    } else if (action == EMIT) {
      /* only emit the C code */
      if (verbose)
        printf("info: emit C code.\n");
      parse = parser_init(code, output, 0, 0);
    } else {
      /* get temp file for generated C code */
      if (verbose)
        printf("info: compile code.\n");
      tmp = mktemp(tmpdefault);
      scat(tmp, ".c");

      if (verbose)
        printf("info: created temp file '%s'\n", tmp);

      parse = parser_init(code, tmp, 0, 0);
    }

    parser_program(&parse);
    parser_fin(&parse);

    if (action == COMPILE) {
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
