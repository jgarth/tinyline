PEDANTIC_FLAGS=-Wall -Wextra -pedantic
CFLAGS=-std=c99 -g

.PHONY: example

example: tinyline.c example.c
	$(CC) tinyline.c example.c -o example $(CFLAGS)

pedantic_example:
	$(CC) tinyline.c example.c -o example $(CFLAGS) $(PEDANTIC_FLAGS)

test: example
	./test
