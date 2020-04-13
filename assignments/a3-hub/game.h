#ifndef __GAME_H__
#define __GAME_H__

/* Standard Libraries */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

#include "2310util.h"

/* Static Definitions */

// general
#define SUITS "SCDH"
#define ALPHA_DIGITS "abcdef"

// constants
#define MIN_ARGS 4
#define MIN_THRESHOLD 2
#define MIN_PLAYERS 2

// messages to players
#define MSG_HAND "HAND%d"
#define MSG_HAND_CONCAT ",%c%x"
#define MSG_ROUND "NEWROUND%d\n"
#define MSG_PLAYED "PLAYED%d,%c%x\n"
#define MSG_GAMEOVER "GAMEOVER\n"
#define MSG_PLAY "PLAY"
#define MSG_PLAY_OUT "PLAY%s\n"

// output
#define OUT_LEAD "Lead player=%d\n"
#define OUT_CARDS "Cards="
#define OUT_CARDS_PLACED "%c.%x "
#define OUT_SCORES "%d:%d "

#define ERR_LEAD "Lead player=%d: "
#define ERR_CARDS "%c.%x"

/* Data Types */

// A data struct to represent a card
typedef struct {
    char suit;
    long rank;
    bool played;
} Card;

// This player's move (playerNo == -1) ||
//      Other player's move (cardIndex == -1)
struct Move {
    int cardIndex;
    int playerNo;
    Card card;
};

// A linked list of all players (pipe clients)
struct Client {
    struct Client* next;
    PipeClient* player;
    int playerID;
    Card* hand;
    bool alive;
    int handSize;
};

// A struct holding most of the game data
typedef struct {
    int noPlayers;
    struct Client* players;
    int cards;
    struct LList* deck;
    struct LList* deckIndex;
    int roundNo;
    struct LList* round;
    int threshold;
} Game;

/* Shared Functions */

// Handling cards
bool is_card(char* rawCard);
Card new_card(char* rawCard);
bool has_card(struct Client* player, char* rawCard);

// Comms channels
void deal_card(Card* card, PipeClient* player);

// print out
void print_lead(int leadID);
void print_cards(Card* cards, int nPlayers);
void print_scores(int* scores, int nPlayers);
void print_round(int leadID, struct LList* cardsPlayed);

// main game
Game* new_round(Game* game, int lead);
int top_score(struct LList* round, int lead, int players);

// clean up
void kill_players(struct Client* players);
void game_kill(Game* game, char* output);
void game_over(Game* game);

#endif
