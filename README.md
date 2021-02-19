# yeti
Simple [Brainfuck](https://esolangs.org/wiki/Brainfuck) compiler written as an educational project based on [Let's make a Teeny Tiny compiler](http://web.eecs.utk.edu/~azh/blog/teenytinycompiler1.html) by Austin Z. Henley.

It uses `gcc` to internally compile and includes all the parts mentioned in Austin's compiler (lexer, parser, emitter) along with the final compiler.

## Build
Just type `make`. That's it.

### TODO
- [ ] Ditch `gcc` and make yeti compile the final binary.
- [ ] Include an interpreter skip compilation process.
