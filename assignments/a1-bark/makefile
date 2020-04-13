# Default styled Makefile for C projects with GNU99

# WARNING: There WILL be collisions with my 2018 version, you can't bake a
# cake without first having the ingredients.

# The primary assembly flags
CFLAGS =-Wall -pedantic -std=gnu99 -g
# CFLAGS +=$(INCLUDE)

# Literally ALL of the debugging flags:
# -Wextra -Wcast-align -Wpointer-arith -Wbad-function-cast -Wmissing-prototypes
# -Wstrict-prototypes -Wmissing-declarations -Winline -Wundef -Wnested-externs
# -Wcast-qual -Wshadow -Wwrite-strings -Wno-unused-parameter -Wfloat-equal
# -pedantic -ansi

PRIMARY =bark
SECONDARY =deck.o player.o score.o
UTILITY =exit.o 2310util.o

# External libraries: include -I[headers], -L[source] -flag
INCLUDE =-lm
# LIBRARY =

# Default assembly process
.PHONY: all
.DEFAULT_GOAL: all
all: $(PRIMARY)

# The main
bark: bark.o $(UTILITY) $(SECONDARY)
	gcc $(UTILITY) $(SECONDARY) $(INCLUDE) bark.o -o bark

bark.o:
	gcc $(CFLAGS) -c bark.c

# Secondaries
player.o: player.c
	gcc $(CFLAGS) -c player.c player.h

deck.o: deck.c
	gcc $(CFLAGS) -c deck.c deck.h

score.o: score.c
	gcc $(CFLAGS) -c score.c score.h

# Utilities
exit.o: exit.c
	gcc $(CFLAGS) -c exit.c exit.h

2310util.o: 2310util.c
	gcc $(CFLAGS) -c 2310util.c 2310util.h

# Clean all assembled files
.PHONY: clean
clean:
	rm bark *.o *.gch

# Debugging flags && additional error calls; -O{1:3} to compress
.PHONY: debug
# debug: CFLAGS +=-O2
# debug: CFLAGS +=-O3
debug: CFLAGS +=-Wextra -Wpointer-arith -Wmissing-prototypes -Werror
debug: all
