#!/bin/sh
# Visualize V7 AST
# Usage examples:
#   ./tests/js2dot.sh -e '1 + 2 * (function(){return 3})();'
#   ./tests/js2dot.sh my_file.js

V7=$(dirname $0)/../v7
TMP_AST=/tmp/._$$.ast
TMP_DOT=/tmp/._$$.dot
TMP_PNG=/tmp/._$$.png

trap "rm -rf $TMP_DOT $TMP_PNG $TMP_AST" EXIT

test -z "$1" && echo "Usage: $0 <js_file|-e expr>" && exit 1;

$V7 -t "$@" | sed '/\/\*/d' > $TMP_AST
echo "graph G {" > $TMP_DOT
echo "node [shape=box];" >> $TMP_DOT
cat $TMP_AST | perl -nle '
  my ($indent) = ($_ =~ /^(\s+)/);
  my $level = length($indent) / 2;   # calculate indentation level
  s/"//g;       # Remove double quotes
  s/^\s+//g;    # Strip leading spaces
  $stack[$level] = $line_no++;    # Use line number as node "identifier"
  print "$stack[$level] [label=\"$_\"]";
  print("  $stack[$level - 1] -- $stack[$level]") if ($level > 0);
' >> $TMP_DOT
echo "}" >> $TMP_DOT

#cat $TMP_AST
#cat $TMP_DOT

dot -Tpng $TMP_DOT -o $TMP_PNG
open $TMP_PNG
sleep 1  # Give 1 second to open generated file before we delete it on exit
