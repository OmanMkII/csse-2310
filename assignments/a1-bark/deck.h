#ifndef __DECK_H__
#define __DECK_H__

/* Libraries */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "2310util.h"

/* Data Types */

typedef struct {
    char suit;
    char value;
} Card;

typedef struct {
    Card** deck;
    char** suits;
    int cards;
} Deck;

struct DeckHolder {
    struct LList* deckList;
    struct LList* deckIndex;
    int deckSize;
};

/* Public Prototypes */

struct DeckHolder* read_deck(char* filepath, int deckIndex);
int get_deck_size(struct LList* deck);
Card* new_card(char* rawCard);

#endif
