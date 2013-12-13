all:
	$(CC) ejs.c -o ejs -DEJS_UNIT_TEST -W -Wall
	./ejs
