

src/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

htdrill: src/htdrill.o src/goaccess/parser.o
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -f src/**/*.o