---
title: V7 internals
items:
  - { type: file, name: nan-packing.md }
  - { type: file, name: mbufs.md }
  - { type: file, name: strings.md }
  - { type: file, name: tokenizer.md }
  - { type: file, name: parser.md }
  - { type: file, name: ast.md }
  - { type: file, name: vm.md }
  - { type: file, name: gc.md }
---

V7 has four loosely coupled layers:

- Tokenizer. Reads input source and returns the next token.
- Parser. Calls Tokenizer to parse the source code and builds an AST.
- Abstract Syntax Tree (AST). A representation of a program's syntax tree
  in a compact and portable binary form.
- Interpreter. An actual VM that executes AST.

Each layer is described in a respective section below.
