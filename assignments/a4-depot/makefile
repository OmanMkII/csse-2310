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

PRIMARY =2310depot
SECONDARY =
UTILITY =2310util.o 2310list.o 2310items.o

# External libraries: include -I[headers], -L[source] -flag
INCLUDE =-lm -lpthread
# LIBRARY =

# Default assembly process
.PHONY: all
.DEFAULT_GOAL: all
all: $(PRIMARY) $(SECONDARY)

# The main depot
2310depot: 2310depot.o $(UTILITY)
	gcc $(UTILITY) $(INCLUDE) 2310depot.o -o 2310depot

2310depot.o:
	gcc $(CFLAGS) -c 2310depot.c

# Utilities
2310util.o: 2310util.c
	gcc $(CFLAGS) -c 2310util.c 2310util.h

2310list.o: 2310list.c
	gcc $(CFLAGS) -c 2310list.c 2310list.h

2310items.o: 2310items.c
	gcc $(CFLAGS) -c 2310items.c 2310items.h

# Clean all assembled files
.PHONY: clean
clean:
	rm $(PRIMARY) *.o *.gch

# Debugging flags && additional error calls; -O{1:3} to compress
# Wno-implicit-fallthrough for local, comment for final submission
.PHONY: debug
# debug: CFLAGS +=-Wno-implicit-fallthrough
debug: CFLAGS +=-Wextra -Wpointer-arith -Wmissing-prototypes -Werror
debug: all
