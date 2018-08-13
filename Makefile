GAMOD= src/goaccess/parser.o src/goaccess/xmalloc.o src/goaccess/util.o src/goaccess/error.o \
 src/goaccess/browsers.o src/goaccess/opesys.o src/goaccess/gkhash.o src/goaccess/gslist.o \
 src/goaccess/gstorage.o src/goaccess/settings.o

CFLAGS=-g -Wall # -D_FORTIFY_SOURCE=2 -O2

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

htdrill: src/htdrill.o src/options.o src/spec_parser.o src/table_print.o $(GAMOD)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f src/**/*.o src/*.o