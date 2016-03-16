---
title: V7 internals
---

V7 has four loosely coupled layers:

- Tokenizer. Reads input source and returns next token.
- Parser. Calls tokenizer to parse the source code, and builds an AST
- Abstract Syntax Tree (AST) - a representation of a program's syntax tree
  in a compact and portable binary form
- Interpreter - an actual VM that executes AST

Each layer is described in a respective section below.
