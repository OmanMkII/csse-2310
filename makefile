# Default styled Makefile for C projects with GNU99

# WARNING: There WILL be collisions with my 2018 version, you can't bake a
# cake without first having the ingredients.

# The primary assembly flags
CFLAGS =-Wall -pedantic -std=gnu99 -g
CFLAGS +=$(INCLUDE)

# Literally ALL of the debugging flags:
# -Wextra -Wcast-align -Wpointer-arith -Wbad-function-cast -Wmissing-prototypes
# -Wstrict-prototypes -Wmissing-declarations -Winline -Wundef -Wnested-externs
# -Wcast-qual -Wshadow -Wwrite-strings -Wno-unused-parameter -Wfloat-equal
# -pedantic -ansi

A1 =a1-bark
LIB =lib

PRIMARY =A1/bark
SECONDARY =A1/deck.o A1/player.o A1/score.o
UTILITY =A1/exit.o LIB/2310util.o

# External libraries: include -I[headers], -L[source] -flag
INCLUDE =-lm -I "../lib/"
LIBRARY =-L "../lib"

# Default assembly process
.PHONY: all
.DEFAULT_GOAL: all
all: $(PRIMARY)

# The main
A1/bark: A1/bark.o $(UTILITY) $(SECONDARY)
	gcc $(UTILITY) $(SECONDARY) $(INCLUDE) A1/bark.o -o A1/bark

A1/bark.o:
	gcc $(CFLAGS) $(LIBRARY) -c *a1*/bark.c

# Secondaries
A1/player.o: A1/player.c
	gcc $(CFLAGS) $(LIBRARY) -c A1/player.c A1/player.h

A1/deck.o: A1/deck.c
	gcc $(CFLAGS) $(LIBRARY) -c A1/deck.c A1/deck.h

A1/score.o: A1/score.c
	gcc $(CFLAGS) -c A1/score.c A1/score.h

# Utilities
A1/exit.o: A1/exit.c
	gcc $(CFLAGS) -c A1/exit.c A1/exit.h

LIB/a1-2310util.o: LIB/a1-2310util.c
	gcc $(CFLAGS) -c LIB/a1-2310util.c LIB/a1-2310util.h

# Clean all assembled files
.PHONY: clean
clean:
	rm A1/bark A1/*.o A1/*.gch LIB/*.o LIB/*.gch

# Debugging flags && additional error calls; -O{1:3} to compress
.PHONY: debug
# debug: CFLAGS +=-O2
# debug: CFLAGS +=-O3
debug: CFLAGS +=-Wextra -Wpointer-arith -Wmissing-prototypes -Werror
debug: all
