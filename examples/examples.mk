SOURCES = $(PROG).c main.c ../../v7.c
CFLAGS = -W -Wall -I../.. $(CFLAGS_EXTRA) $(MODULE_CFLAGS)

all: $(PROG)

$(PROG): $(SOURCES)
	$(CC) $(SOURCES) -o $@ $(CFLAGS) -lm

$(PROG).exe: $(SOURCES)
	cl $(SOURCES) /I../.. /MD /Fe$@

clean:
	rm -rf *.gc* *.dSYM *.exe *.obj *.o a.out $(PROG)
