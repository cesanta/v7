# This example shows that V7 can be embedded in a DLL
# both on unices and on windows. This example has the same source code
# as the `call_js_from_c` example; it just shows that V7 works just
# fine if linked inside a dynamic library.
#
# NOTE: We do not provide a v7.def file, so if you want to expose
# the bare v7 as a DLL on windows you need to export the API manually.
# We could include the v7.def file in our main repository if there is
# sufficient demand; however that's not the expected use case of v7
# so we don't currently bother maintaining an export definition for
# windows. It's more likely that you develop a DLL and within it you need
# some scripting.

PROG = call_js_from_c

SOURCES = ../$(PROG)/$(PROG).c ../../v7.c
MAIN = ../$(PROG)/main.c
CFLAGS = -W -Wall -I../.. $(CFLAGS_EXTRA) $(MODULE_CFLAGS)

WINE ?=
ifneq ($(OS),Windows_NT)
WINE = wine
endif

ifeq ($(OS),Windows_NT)
all: main.exe
else
all: main
endif

lib$(PROG).so: Makefile $(SOURCES)
	$(CC) -o $@ $(CFLAGS) -shared -fpic $(SOURCES)

main: Makefile $(SOURCES) lib$(PROG).so
	$(CC) $(MAIN) -o $@ $(CFLAGS) -lm -l$(PROG) -L.

$(PROG).dll: Makefile $(SOURCES)
	$(WINE) cl /c /I../.. /MD $(SOURCES)
	$(WINE) link /DLL /DEF:$(PROG).def \
		 $(notdir $(subst .c,.obj,$(SOURCES))) \

main.exe: Makefile $(MAIN) $(PROG).dll
	$(WINE) cl $(MAIN) /MD /Fe$@ /link $(PROG).lib

clean:
	rm -rf *.gc* *.dSYM *.exe *.obj *.o a.out *.so *.dll *.exp *.lib $(PROG) main
