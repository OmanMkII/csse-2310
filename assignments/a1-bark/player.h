#ifndef __PLAYER_H__
#define __PLAYER_H__

/* Libraries */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "deck.h"
#include "exit.h"

/* Static Def'ns */

#define HAND_SIZE 6
// <score><suit><terminator>
#define CARD_SIZE 3

#define HUMAN 'h'

// #define H_HAND "Hand(%d): %s %s %s %s %s %s\n"
#define H_HAND "Hand(%d):"
#define H_MOVE "Move? "

#define H_MOVE_ARGS 3
#define TTL_IGNORE -1

// save with "SAVE<path>"
#define SAVE "SAVE"

#define AUTO 'a'

// #define A_HAND "Hand: %s %s %s %s %s %s\n"
#define A_HAND "Hand:"
#define A_PLAY "Player %d plays %s in column %d row %d\n"

#define HAND_BUFFER " %s"

/* Data Types */

typedef struct {
    char* card;
    int* move;
    // TTL for knowing first move && in case of infinite looping
    // if human, ttl = -1, else ttl = w * h + 1 && decrementing
    int ttl;
    bool save;
    char* savepath;
} Move;

/* Visible Prototypes */

// Build and destroy player
Player* new_player(char* type);
void free_player(Player* target);

// Manage turns
Move* make_turn(Player* player, Board* board, Move* last, int playerNo);

// Manage hands
Player* add_card(Player* player, char* card);
Player* remove_card(Player* player, char* card);

#endif
