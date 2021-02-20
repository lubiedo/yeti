# yeti
Simple [Brainfuck](https://esolangs.org/wiki/Brainfuck) compiler written as an educational project based on [Let's make a Teeny Tiny compiler](http://web.eecs.utk.edu/~azh/blog/teenytinycompiler1.html) by Austin Z. Henley.

It uses `gcc` to internally compile and includes all the parts mentioned in Austin's compiler (lexer, parser, emitter) along with the final compiler. It can also interpret bf code in order to compile and produce an output on the fly without generating any kind of executable nor code.

## Build
Just type `make`. That's it.

### TODO
- [ ] Ditch `gcc` and make yeti compile the final binary.
- [X] Include an interpreter skip compilation process.

### TOFIX
- [ ] Child can't read from STDIN when interpreted.
