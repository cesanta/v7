---
title: Tokenizer
---

Tokenizer reads input source and returns the next token. V7 uses hand-written
tokenizer. All possible tokens are described as `enum v7_tok`. Tokenizer
exports following interface:

#### Skip to next token

```c
int skip_to_next_tok(const char **ptr);
```

Move source code pointer `ptr` to the next token, skipping whitespaces
and comments. Return number of new lines skipped.

#### Skip to next token

```c
enum v7_tok get_tok(const char **s, double *n, enum v7_tok prev_tok);
```

Return a token pointed by source code pointer `s`. `get_tok()` expects
`skip_to_next_tok()` called, i.e. that `s` points to the valid next token.
If the returned token is `TOK_NUMBER`, then `n` will contain the parsed number.
`prev_tok` is required to parse RegExp literals, otherwise it would be
impossible to correctly parse strings like: `a /= b; c /= d;`: when the
tokenizer sees first `/`, it wouldn't know if it is a division or a
regexp literal.

`get_tok()` is basically a giant `switch` statement that looks at the first
character of the input string. For each letter, there is a short number of
tokens that begins with that letter, and `get_tok()` iterates over all of them.
Usually compilers generate jump table for the `switch` statement, which makes
`get_tok()` quite fast.

#### Is reserved word

```c
int is_reserved_word_token(enum v7_tok tok);
```

Return non-zero if the token is one of the reserved words. To make this
function fast, all reserved words tokens are grouped together in alphabetical
order, thus enabling this implementation:

```c
int is_reserved_word_token(enum v7_tok tok) {
  return tok >= TOK_BREAK && tok <= TOK_WITH;
}
```
