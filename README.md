# yeti
Simple [Brainfuck](https://esolangs.org/wiki/Brainfuck) compiler written as an educational project based on [Let's make a Teeny Tiny compiler](http://web.eecs.utk.edu/~azh/blog/teenytinycompiler1.html) by Austin Z. Henley.

It uses `gcc` to internally compile and includes all the parts mentioned in Austin's compiler (lexer, parser, emitter) along with the final compiler. It can also *interpret* bf code in order to compile and produce an output on the fly without generating any kind of executable nor code.
It comes with a *debugger* in order to debug the code and memory and follow the flow of your bf code.

## Build
Just type `make`. That's it.

### Notes
Check the `test/` folder in order to get some examples on how to use the codebase.

### TODO
- [ ] Ditch `gcc` and make yeti compile the final binary.
- [X] Include an interpreter skip emitting process.
- [ ] More exhaustive work on the interpreter (eg. test against more complex code).

### TOFIX
- [X] Child can't read from STDIN when interpreted.
