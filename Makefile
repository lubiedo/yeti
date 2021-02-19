CC = gcc
CFLAGS 	= -Wall -pedantic -O2 -g

all: build build_test

build: $(subst .c,,$(wildcard *.c))
%: %.c
	$(CC) $(CFLAGS) -c $< -o $@.o
yeti: yeti.c
	$(CC) $(CFLAGS) $< -o $@ *.o

build_test:
	( cd test/ && make 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' all )

.PHONY: clean
clean:
	rm -rf yeti *.o *.dSYM
	find . -name a.out -delete
	cd test/ && make clean
