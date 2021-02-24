#include <stdlib.h>
#include <unistd.h>

#define MAX_BUFF  256
#define MAIN_INIT "#include <stdio.h>\n" \
  "#include <string.h>\n" \
  "int main(){\n" \
  "char mem[30000];\n" \
  "char *ptr = mem;\n" \
  "memset(mem, 0, 30000);\n" /* clean mem */
#define MAIN_FIN  "return 0;\n" \
  "}\n"

/* string.h replacements */
static void __attribute__((unused))
clean(char *s, size_t l)
{
  while(l-- > 0)
    *(s+l) = '\0';
}

static char * __attribute__((unused))
scpy(char *d, char *s)
{
  char *p = d;

  if (*s == '\0')
    return NULL;
  while(*s != '\0')
    *p++ = *s++;
  return d;
}

static char * __attribute__((unused))
scat(char *d, char *s)
{
  char *p = d;

  if (*s == '\0')
    return NULL;
  while (*++p != '\0') ;
  while(*s != '\0')
    *p++ = *s++;
  return d;
}

static size_t __attribute__((unused))
slen(char *s)
{
  size_t l = 0;

  if (*s == '\0')
    return l;
  while(*s++ != '\0')
    l++;
  return l;
}

static int __attribute__((unused))
gchar()
{
  int c;
  read(0, (void *)&c, 1);
  return c;
}

static int __attribute__((unused))
pchar(int c)
{
  write(1, (void *)&c, 1);
  return c;
}

static void __attribute__((unused))
_abort(const char *err, const int pos)
{
  if (err != NULL) {
    if (pos > -1)
      fprintf(stderr, "error: %s (at pos: %d)\n", err, pos);
    else
      fprintf(stderr, "error: %s\n", err);
  }

  exit(EXIT_FAILURE);
}

/* testing
 * taken from: https://esolangs.org/wiki/Brainfuck
 */
#define HELLO_WORLD "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++."
#define CAT ",[.,]"

/* lexer.c */
enum {
  TOKEN_EOF   = -1,
#define CHAR_EOF    '\0'
  TOKEN_UNKNOWN,
  TOKEN_RIGHT,      /* > */
#define CHAR_RIGHT  '>'
  TOKEN_LEFT,       /* < */
#define CHAR_LEFT   '<'
  TOKEN_INC,        /* + */
#define CHAR_INC    '+'
  TOKEN_DEC,        /* - */
#define CHAR_DEC    '-'
  TOKEN_OUT,        /* . */
#define CHAR_OUT    '.'
  TOKEN_IN,         /* , */
#define CHAR_IN     ','
  TOKEN_OLOOP,      /* [ */
#define CHAR_OLOOP  '['
  TOKEN_CLOOP       /* ] */
#define CHAR_CLOOP  ']'
};
struct lexer {
  const char *src;
  int pos;
};
extern struct lexer lexer_init(const char *s);
extern        int   lexer_token(struct lexer *l);
extern        void  lexer_print(int token);

/* emit.c */
struct emitter {
  int fd;
  char code[MAX_BUFF];
};
extern struct emitter emitter_init(const char *path);
extern        int     emitter_fin(struct emitter *e);
extern        ssize_t emitter_out(struct emitter *e);

/* interpreter.c */
#define MEMSIZE  30000
struct interpreter {
  char *ptr;
  char mem[MEMSIZE];
};
extern struct interpreter interpreter_init(const int memlen);
extern        void        interpreter_right(struct interpreter *i);
extern        void        interpreter_left(struct interpreter *i);
extern        void        interpreter_inc(struct interpreter *i);
extern        void        interpreter_dec(struct interpreter *i);
extern        void        interpreter_in(struct interpreter *i);
extern        void        interpreter_out(struct interpreter *i);

/* parser.c */
struct parser {
  struct lexer        lexer;
  struct emitter      emitter;
  struct interpreter  interpreter;
  int interpreting;
};
extern struct parser  parser_init(const char *s, const char *o, const int vm);
extern        int     parser_fin(struct parser *p);
extern        void    parser_program(struct parser *p);
