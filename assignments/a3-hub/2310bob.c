/* Libraries */

#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "player.h"

/* Static Definitions */

// all hail the First Order
#define FIRST_ORDER "DHSC"
#define SECOND_ORDER "SCHD"
#define THIRD_ORDER "SCDH"

/* Local Prototypes */

struct Query* query_hand(struct Hand hand, char suit);
bool lead_move(Player* hand);
bool near_threshold(Player* playerData);
bool threshold_move(Player* hand, char leadSuit);
void my_move(Player* player, bool leadPlayer, char leadSuit);

/* Main */

int main(int argc, char** argv) {
    // build Player
    Player* bob = new_player(argc, argv);
    bool isNewGame = true;
    // play the game
    while(!bob->gameover) {
        bob = make_play(bob, &my_move, isNewGame);
        isNewGame = false;
    }
    // free data & return
    free(bob);
    return 0;
}

/* Local Functions */

/* Reads Bob's hand and makes best move as the lead player, returns true if a
 * move was made, else false signifies error.
 */
bool lead_move(Player* player) {
    struct Query* query = NULL;
    int i = 0;
    while(i < (int)strlen(FIRST_ORDER)) {
        query = query_hand(player->hand, FIRST_ORDER[i]);
        if(query != NULL) {
            break;
        }
        i++;
    }
    // check there's a valid move
    if(query == NULL) {
        printf("DEBUG: no valid suits (play 1st)\n");
        return false;
    }
    // play highest rank
    player = play_card(player, query->lowest);
    free(query);
    return true;
}

/* Checks if any players (including this one) are at close to the threshold,
 * specifically >= (T - 2); used to initialise on first check.
 */
bool near_threshold(Player* playerData) {
    // if not yet initialised
    if(playerData->dScores.count == NULL) {
        playerData->dScores.count = malloc(sizeof(int) * playerData->players);
        for(int i = 0; i < playerData->players; i++) {
            playerData->dScores.count[i] = 0;
        }
        // checks if not already there
        return !(playerData->threshold > 2);
    }
    // else check arrays
    for(int i = 0; i < playerData->players; i++) {
        if(playerData->dScores.count[i] >= playerData->threshold - 2) {
            return true;
        }
    }
    // ~(two or less required)
    return !(playerData->threshold > 2);
}

/* Reads Bob's hand and makes best move in case 2, returns true if made, else
 * false to signify error.
 */
bool threshold_move(Player* player, char leadSuit) {
    struct Query* query = query_hand(player->hand, leadSuit);
    if(query != NULL) {
        // case 2.1: have lead suit
        player = play_card(player, query->highest);
    } else {
        // case 2.2: else in second order
        for(int i = 0; i < (int)strlen(SECOND_ORDER); i++) {
            query = query_hand(player->hand, SECOND_ORDER[i]);
            if(query != NULL) {
                player = play_card(player, i);
                return true;
            }
        }
    }
    return false;
}

/* Takes Bob's current hand, and if they are lead | the lead suit, and makes
 * a move from the data given.
 */
void my_move(Player* player, bool leadPlayer, char leadSuit) {
    if(leadPlayer) {
        // case 1: lead player (use 1st order)
        bool played = lead_move(player);
        if(!played) {
            printf("DEBUG: bad move from Bob\n");
        }
    } else if(near_threshold(player) && player->round.dCardPlayed) {
        // case 2: N(players with D cards >= {threshold - 2}) > 0 &&
        //      N(D cards) > 0 this round (use 2nd order)
        bool played = threshold_move(player, leadSuit);
        if(!played) {
            printf("DEBUG: bad move from Bob\n");
        }
    } else {
        // neither 1 not 2
        struct Query* query = NULL;
        // case 3: have a card in the lead suit
        query = query_hand(player->hand, leadSuit);
        if(query != NULL) {
            player = play_card(player, query->highest);
            free(query);
        }
        // case 4: otherwise (use 3rd order)
        for(int i = 0; i < (int)strlen(THIRD_ORDER); i++) {
            query = query_hand(player->hand, THIRD_ORDER[i]);
            if(query != NULL) {
                player = play_card(player, query->highest);
                free(query);
                break;
            }
        }
    }
}
