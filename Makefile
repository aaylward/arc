all: arc

arc: arc.c
	$(CC) arc.c -o arc -Wall -Wextra -pedantic -std=c17

clean:
	rm arc
