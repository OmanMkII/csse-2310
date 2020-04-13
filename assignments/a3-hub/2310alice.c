/* Libraries */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "game.h"
#include "player.h"

/* Static Definitions */

// all hail the First Order
#define FIRST_ORDER "SCDH"
#define OTHER_ORDER "DHSC"

/* Local Prototypes */

void my_move(Player* player, bool leadPlayer, char leadSuit);

/* Main */

int main(int argc, char** argv) {
    // build Player
    Player* alice = new_player(argc, argv);
    bool isNewGame = true;
    // play the game
    while(!alice->gameover) {
        alice = make_play(alice, &my_move, isNewGame);
        isNewGame = false;
    }
    // free data & return
    free(alice);
    return 0;
}

/* Local Functions */

/* Takes Alice's current hand and prints her moves, dependent on her being the
 * lead player, else the leading suit of the last player.
 */
void my_move(Player* player, bool leadPlayer, char leadSuit) {
    if(leadPlayer) {
        // Case 1: first player
        struct Query* data = NULL;
        int i = 0;
        // while(data == NULL && i < 4) {
        while(i < (int)strlen(FIRST_ORDER)) {
            data = query_hand(player->hand, FIRST_ORDER[i]);
            if(data != NULL) {
                break;
            }
            i++;
        }
        // check there's a valid move
        if(data == NULL) {
            printf("DEBUG: no valid suits (play 1st)\n");
            fflush(stdout);
        }
        // play highest rank
        player = play_card(player, data->highest);
        // player->hand.cards[data->highest].played = true;
        free(data);
    } else {
        // Case 2: not first player
        struct Query* data = query_hand(player->hand, leadSuit);
        // if hand contains instance in lead suit
        // play lowest card
        if(data != NULL) {
            player = play_card(player, data->lowest);
            // player->hand.cards[data->lowest].played = true;
            free(data);
        } else {
            // Case 3: not first, no same suit
            int i = 0;
            while(data == NULL && i < (int)strlen(OTHER_ORDER)) {
                data = query_hand(player->hand, OTHER_ORDER[i]);
                i++;
            }
            if(data == NULL) {
                printf("DEBUG: no valid suits (play non-1st)\n");
                printf("TODO: what do here?\n");
                fflush(stdout);
            }
            // play highest rank
            player = play_card(player, data->highest);
            // player->hand.cards[data->highest].played = true;
            free(data);
        }
    }
}
