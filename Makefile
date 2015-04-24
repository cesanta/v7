WARNS = -W -Wall -pedantic -Wno-comment -Wno-variadic-macros -Wno-unused-function
#V7_FLAGS = -I.
CFLAGS = $(WARNS) -g -O3 -lm $(V7_FLAGS) $(CFLAGS_PLATFORM) $(CFLAGS_EXTRA)

.PHONY: examples test

all: examples test v7

examples:
	@$(MAKE) -C examples

tests/unit_test: tests/unit_test.c v7.c Makefile
		$(CC) tests/unit_test.c -DV7_EXPOSE_PRIVATE -DV7_UNIT_TEST $(CFLAGS) -lm -o tests/unit_test

test: tests/unit_test
	cd tests; ./unit_test $(TEST_FILTER)

v7: v7.c v7.h Makefile
	$(CC) v7.c -o $@ -DV7_EXE $(CFLAGS) -lm

clean:
	@$(MAKE) -C examples clean
	rm -fr v7 v7.dSYM tests/unit_test

