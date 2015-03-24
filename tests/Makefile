PROG = unit_test
SRC_DIR=../src
WARNS = -W -Wall -Wno-comment -Wno-variadic-macros
V7_FLAGS = -I$(SRC_DIR) -I. -DV7_EXPOSE_PRIVATE
CFLAGS = $(WARNS) -g -O3 $(V7_FLAGS) $(CFLAGS_PLATFORM) $(CFLAGS_EXTRA)
LDFLAGS = -lm
AMALGAMATED_SOURCES = ../v7.c

include test.mk
include $(SRC_DIR)/sources.mk

save_want_ast:
	@rm -f unit_test
	$(MAKE) test_c99 TEST_FILTER=parser CFLAGS_EXTRA=-DSAVE_AST
	@rm -f unit_test

# Interactive:
#   docker run -v $(CURDIR)/../..:/cesanta -t -i --entrypoint=/bin/bash cesanta/v7_test
docker:
	docker run --rm -v $(CURDIR)/../..:/cesanta cesanta/v7_test

docker_msan:
	@docker run --rm -v $(CURDIR)/../..:/cesanta -t -i --entrypoint=/bin/bash cesanta/v7_test -c "cd /cesanta/v7; make -C tests test_msan TEST_FILTER=$(TEST_FILTER)"
