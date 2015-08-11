# V7 Embedded JavaScript Library Release Notes

## Release 1.0, 2015-05-26

- Implemented hand-written tokenizer and parser
- Implemented AST-based VM runtime
- Implemented mark-and-sweep garbage collector
- Published examples
- Published [documentation with API reference](https://docs.cesanta.com/v7/)
- Standard Library is partially implemented. Unimplemented API:
    + global: `decodeURI()`, `decodeURIComponent()`
    + Object: `seal()`, `freeze()`, `isSealed()`, `isFrozen()`
    + Function: `bind()`
    + Array: `concat()`, `unshift()`, `forEach()`, `reduceRight()`
    + JSON: `replacer` argument in `stringify()` is ignored
- Auxiliary library is not implemented yet
