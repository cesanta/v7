# V7 Embedded JavaScript Library Release Notes

## Release 1.0, 2015-05-25

- Implemented core components of the engine:
    + Tokenizer
    + Parser (Abstract Syntax Tree generator)
    + Interpreter
    + Garbage collector
- Published C/C++ API
- Published examples
- Standard Library is partially implemented. Unimplemented API:
    + `parseInt()`, `parseFloat()`, `isFinite()`
    + `decodeURI()`, `decodeURIComponent()`
    + `Object.seal()`, `Object.freeze()`, `Object.isSealed()`, `Object.isFrozen()`
    + `Function.length`, `Function.prototype.bind()`
- ECMA tests coverage XX.XX%
