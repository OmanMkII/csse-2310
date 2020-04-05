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

PRIMARY =2310hub
SECONDARY =2310alice 2310bob
UTILITY =game.o player.o 2310util.o

# External libraries: include -I[headers], -L[source] -flag
INCLUDE =-lm
# LIBRARY =

# Default assembly process
.PHONY: all
.DEFAULT_GOAL: all
all: $(PRIMARY) $(SECONDARY)

# The main
2310hub: 2310hub.o $(UTILITY)
	gcc $(UTILITY) $(INCLUDE) 2310hub.o -o 2310hub

2310hub.o:
	gcc $(CFLAGS) -c 2310hub.c

# Secondary Clients

2310alice: 2310alice.o $(UTILITY)
	gcc $(UTILITY) $(INCLUDE) 2310alice.o -o 2310alice

2310alice.o:
	gcc $(CFLAGS) -c 2310alice.c

2310bob: 2310bob.o $(UTILITY)
	gcc $(UTILITY) $(INCLUDE) 2310bob.o -o 2310bob

2310bob.o:
	gcc $(CFLAGS) -c 2310bob.c

# Utilities
player.o: player.c
	gcc $(CFLAGS) -c player.c player.h

game: game.o
	gcc $(CFLAGS) -c game.c game.h

2310util.o: 2310util.c
	gcc $(CFLAGS) -c 2310util.c 2310util.h

# Clean all assembled files
.PHONY: clean
clean:
	rm $(PRIMARY) $(SECONDARY) *.o *.gch

# Debugging flags && additional error calls; -O{1:3} to compress
# Wno-implicit-fallthrough for local, comment for final submission
.PHONY: debug
# debug: CFLAGS +=-Wno-implicit-fallthrough
debug: CFLAGS +=-Wextra -Wpointer-arith -Wmissing-prototypes -Werror
debug: all
