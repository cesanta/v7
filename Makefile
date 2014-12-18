WARNS = -W -Wall -pedantic -Wno-comment -Wno-variadic-macros
V7_FLAGS = -I./src -I.
CFLAGS = $(WARNS) -g -O0 -lm $(PROF) $(V7_FLAGS) $(CFLAGS_EXTRA)
SOURCES = src/global_vars.c src/util.c src/crypto.c src/array.c src/boolean.c \
          src/date.c src/error.c src/function.c src/math.c src/number.c \
          src/object.c src/regex.c src/rune.c src/runetype.c src/string.c \
          src/json.c src/stdlib.c src/parser.c src/tokenizer.c src/api.c \
          src/ast.c src/vm.c src/aparser.c src/main.c
HEADERS = src/v7_license.h src/utf.h src/tokenizer.h src/vm.h src/internal.h \
					src/ast.h src/aparser.h

.PHONY: cpplint

all: v7 amalgamated_v7 unit_test

v7.c: $(HEADERS) $(SOURCES) v7.h Makefile
	cat v7.h $(HEADERS) $(SOURCES) | sed -E '/#include .*(v7|utf|tokenizer|vm|ast|aparser|internal).h"/d' > $@

v: unit_test
	valgrind -q --leak-check=full --show-reachable=yes \
	--leak-resolution=high --num-callers=100 ./unit_test
#	valgrind -q --leak-check=full ./v7 tests/run_tests.js
#	gcov -a unit_test.c

unit_test: $(SOURCES) v7.h tests/unit_test.c src/v7_license.h src/utf.h src/internal.h
	$(CC) $(SOURCES) tests/unit_test.c -o $@ -DV7_PRIVATE="" $(CFLAGS)

xrun: unit_test
	$(CC) -W -Wall -I. -I./src src/tokenizer.c -DTEST_RUN -DV7_PRIVATE= -o t
	./t

run: unit_test
	./unit_test $(TEST_FILTER)

all_warnings: v7.c
	$(CC) v7.c tests/unit_test.c -o $@ -Weverything -Werror $(CFLAGS)
	./$@

v7: src/v7_license.h src/utf.h src/internal.h $(SOURCES) v7.h
	$(CC) $(SOURCES) -o $@ -DV7_EXE -DV7_PRIVATE="" $(CFLAGS) -lm
#	$(CC) $(SOURCES) -o $@ -DV7_EXE $(CFLAGS) -lm

amalgamated_v7: v7.h v7.c
	$(CC) v7.c -o $@ -DV7_EXE -DV7_PRIVATE="" $(CFLAGS) -lm

js: v7
	@./v7 tests/v7_basic_test.js
	@rhino -version 130 tests/v7_basic_test.js

t: v7
	./v7 tests/run_ecma262_tests.js

w: v7.c
	wine cl tests/unit_test.c $(SOURCES) $(V7_FLAGS) /Zi -DV7_PRIVATE=""
	wine unit_test.exe

clean:
	rm -rf *.gc* *.dSYM *.exe *.obj *.pdb a.out u unit_test v7 amalgamated_v7 t

setup-hooks:
	for i in .hooks/*; do ln -s ../../.hooks/$$(basename $$i) .git/hooks; done

difftest:
	@TMP=`mktemp -t checkout-diff.XXXXXX`; \
	git diff v7.c  >$$TMP ; \
	if [ -s "$$TMP" ]; then echo found diffs in checkout:; git status -s;  exit 1; fi; \
	rm $$TMP

cpplint:
	cpplint.py --verbose=0 --extensions=c,h src/*.[ch] v7.h 2>&1 >/dev/null| grep -v "Done processing" | grep -v "file excluded by"

docker:
	docker run --rm -v $(CURDIR)/..:/cesanta cesanta/v7_test
