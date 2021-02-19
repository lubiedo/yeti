TESTS := $(subst .c,,$(wildcard *.c))

all: build
build: $(TESTS)

%: %.c
	$(CC) $(CFLAGS) -I../ -g $< -o $@ ../*.o

.PHONY: clean
clean:
	rm -rf $(TESTS) *.dSYM helloworld.c cat.c
