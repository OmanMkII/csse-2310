#ifndef __EXIT_H__
#define __EXIT_H__

/* Libraries */

#include <stdio.h>
#include <stdlib.h>

#include "deck.h"
// #include "player.h"
#include "2310util.h"

/* Data Types */

// Here for usability: in bark.c I can't use it under exit.*

typedef struct {
    char type;
    struct LList* llHand;
} Player;

typedef struct {
    int* dim;
    // x -> y -> card
    Card*** slots;
} Board;

typedef struct {
    int turn;
    bool first;
    char* deckfile;
    Player** players;
    // int deckSize;
    struct LList* deckIndex;
    struct LList* deck;
    Board* board;
} Bark;

/* Global Types */

typedef enum {
    NORMAL = 0,
    ARGS_NUM = 1,
    ARGS_TYPE = 2,
    FILE_DECK = 3,
    FILE_SAVE = 4,
    SHORT_DECK = 5,
    BOARD_FULL = 6,
    EOINPUT = 7,
} ExitCode;

/* Static Definitions */

// STDOUT Outputs
#define EXIT "Player 1=%d Player 2=%d\n"

// STDERR Outputs
#define ERR_1A "Usage: bark savefile p1type p2type"
#define ERR_1B "bark deck width height p1type p2type"
#define ERR_2 "Incorrect arg types"
#define ERR_3 "Unable to parse deckfile"
#define ERR_4 "Unable to parse savefile"
#define ERR_5 "Short deck"
#define ERR_6 "Board full"
#define ERR_7 "End of input"

/* Public Prototypes */

void exit_prog(int code, Bark* game);

#endif
