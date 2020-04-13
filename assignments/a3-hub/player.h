#ifndef __PLAYER_H__
#define __PLAYER_H__

/* Standard Libraries */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "game.h"
#include "2310util.h"

/* Static Definitions */

// general
#define CLIENT_ARGNUM 5

// Error messages
#define ERR_ARGNUM "Usage: player players myid threshold handsize\n"
#define ERRNO_ARGNUM 1
#define ERR_PLAYERS "Invalid players\n"
#define ERRNO_PLAYERS 2
#define ERR_POSITION "Invalid position\n"
#define ERRNO_POSITION 3
#define ERR_THRESHOLD "Invalid threshold\n"
#define ERRNO_THRESHOLD 4
#define ERR_HANDSIZE "Invalid hand size\n"
#define ERRNO_HANDSIZE 5
#define ERR_MESSAGE "Invalid message\n"
#define ERRNO_MESSAGE 6
#define ERR_EOF "EOF\n"
#define ERRNO_EOF 7

// init complete flag
#define MSG_SETUPCOMPLETE "@"

// HANDn,SR,SR,SR
#define COMM_HAND "HAND"
#define COMM_HAND_NO 0
// NEWROUNDL
#define COMM_NEW_ROUND "NEWROUND"
#define COMM_NEW_ROUND_NO 1
// PLAYEDw,SR
#define COMM_PLAYER "PLAYED"
#define COMM_PLAYER_NO 2
// GAMEOVER
#define COMM_GAMEOVER "GAMEOVER"
#define COMM_GAMEOVER_NO 3
// PLAYSR
#define COMM_PLAY "PLAY"
#define COMM_MAKE_PLAY "PLAY%c%x\n"
#define COMM_PLAY_NO 4
// fpointer offset
#define COMM_OFFSET 1

/* Data Types */

// Query results for hand information
struct Query {
    int highest;
    int lowest;
};

// Player's current hand
struct Hand {
    int size;
    Card* cards;
};

// Player's info on current round and their own status
struct Round {
    int lead;
    int noPlays;
    char leadSuit;
    bool havePlayed;
    bool dCardPlayed;
    struct LList* cardsPlayed;
};

// Player's info on all current threshold scores
struct Scores {
    int* count;
};

// Player's info
typedef struct {
    int players;
    int position;
    int threshold;
    bool gameover;
    struct Round round;
    struct Hand hand;
    struct Scores dScores;
} Player;

/* Communication Data Types:
 *
 * Hub <-> Player communications is text based, which is a real pain to deal
 * with, so these data types handle what was said via struct/union combinations
 * to minimise data load.
 */

// TODO: write function pointers to use these unions

// Union of data input types, managed by parent struct's index
// Function pointers should index by order in this union
union Input {
    // Hub -> Player : N cards && LList(hand)
    // TODO: might be better as an array
    struct Hand playerHand;
    // Hub -> Player : L no. of lead player for this turn
    int leadPlayer;
    // Hub -> Player : player W's move M
    struct Move otherMove;
    // Hub -> Player : Game over state
    bool gameOver;
    // Player -> Hub : this player's Move
    struct Move myMove;
};

// Data type housing anything to/from pipes, holds everything about the last
// move, current state of the game xor the player's next move
typedef struct {
    int inputType;
    union Input input;
} GameState;

/* Shared Functions */

// build new game
Player* new_player(int numArgs, char** inputs);

// play the game
struct Query* query_hand(struct Hand hand, char suit);
void print_last_round(int winner, struct LList* cardsPlayed);
Player* make_play(Player* player, void (*move)(Player*, bool, char),
        bool newGame);
Player* play_card(Player* player, int cardno);

// clean up
void free_player(Player* player);

#endif
