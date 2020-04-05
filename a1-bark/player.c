/* The main Player platform, broken up into automated and user based types */

// header
#include "player.h"

/* Local Prototypes */

// Human: prefix h_(function)
Move* h_move(Player* player);

// Bot: prefix b_(function)
Move* b_move(Player* player, Board* board, Move* last, int playerNo);
Move* b1_move(Player* player, Board* board, Move* last);
Move* b2_move(Player* player, Board* board, Move* last);

// Create new move
Move* new_move(char* card, int x, int y, int ttl);

/* Functions */

/* Creates a new Player struct from the given type */
Player* new_player(char* type) {
    // check input
    if(type == NULL) {
        return NULL;
    } else if(strlen(type) != 1) {
        return NULL;
    } else if(type[0] != HUMAN && type[0] != AUTO) {
        // neither of valid chars
        return NULL;
    }
    // init player
    Player* p = (Player*)malloc(sizeof(Player));
    p->type = type[0];
    p->llHand = llist(EMPTY_STRING);
    return p;
}

/* Sorts whoich type of turn should be made, filtering to automated or CLI
 * prompted for user.
 */
Move* make_turn(Player* player, Board* board, Move* last, int playerNo) {
    // print hand
    if(last == NULL) {
        // Human or autobot
        if(player->type == HUMAN) {
            printf(H_HAND, playerNo);
        } else {
            printf(A_HAND);
        }
        // print cards
        for(struct LList* it = player->llHand; it != NULL; it = it->next) {
            printf(HAND_BUFFER, it->entry);
        }
        printf(NEW_LINE);
    }
    // try moves
    Move* move;
    if(player->type == HUMAN) {
        move = h_move(player);
    } else if(player->type == AUTO) {
        move = b_move(player, board, last, playerNo);
    } else {
        // internal check for problems
        raise(MSG_100, ERROR);
        exit(ERROR);
    }
    // free(last->card);
    // free(last->move);
    // free(last);
    return move;
}

/* Prompts stdin (aka "human") for their move. */
Move* h_move(Player* player) {
    // prompt for move
    char buffer[STRING_BUFFER];
    printf(H_MOVE);
    fgets(buffer, STRING_BUFFER, stdin);
    // check for EOF
    if(feof(stdin)) {
        // printf("DEBUG: EOF check\n");
        return NULL;
    } else if(buffer[0] == SAVE[0] && buffer[1] == SAVE[1] &&
            buffer[2] == SAVE[2] && buffer[3] == SAVE[3]) {
        // char* savedest = buffer + strlen(SAVE);
        // printf("DEBUG: save name %s\n", savedest);
        Move* save = malloc(sizeof(Move));
        save->save = true;
        save->savepath = buffer + strlen(SAVE);
        return save;
    }
    // interpret && return move
    char** args = spltstr(buffer, SPACE);
    // check correct number && type of args
    int len = 0;
    while(args[len] != NULL) {
        // printf("DEBUG: len = %d\n", len);
        len++;
    }
    Move* move = malloc(sizeof(Move));
    move->ttl = -1;
    move->save = false;
    move->move = NULL;
    // error checks
    if(len != 3 || args[len] != NULL) {
        // printf("DEBUG: bad len\n");
        return move;
    } else if(!isnum(args[0]) || !isnum(args[1]) || !isnum(args[2])) {
        // printf("DEBUG: bad arg type\n");
        return move;
    } else if(atoi(args[0]) < 1 || 6 < atoi(args[0])) {
        // printf("DEBUG: bad arg range\n");
        return move;
    }
    free(move);
    // valid
    char* nthCard = lstget(player->llHand, atoi(args[0]) - 1);
    Move* attempt = new_move(nthCard, atoi(args[1]), atoi(args[2]),
            TTL_IGNORE);
    return attempt;
}

/* Manages first moves from either automated player, and splits later moves to
 * appropriate type.
 */
Move* b_move(Player* player, Board* board, Move* last, int playerNo) {
    Move* move;
    if(last == NULL) {
        // Both players start at floor(w + 1 / 2), floor(h + 1 / 2)
        move = new_move(lstget(player->llHand, 0),
                floor((board->dim[0] + 1) / 2),
                floor((board->dim[1] + 1) / 2),
                board->dim[0] * board->dim[1] + 1);
    } else {
        // Else use player's strategy
        if(playerNo == 1) {
            move = b1_move(player, board, last);
        } else {
            move = b2_move(player, board, last);
        }
    }
    move->save = false;
    return move;
}

/* Generate next attempt for auto P1;
 *
 * Player 1 starts searching in the top left corner of the board (0, 0), look
 * across the row (from left to right). If no suitable space is found, move
 * down to the next row an continue searching from the lefthand side.
 */
Move* b1_move(Player* player, Board* board, Move* last) {
    static int origin[2] = {0, 0};
    Move* move;
    if((last->ttl - 1) == (board->dim[0] * board->dim[1])) {
        move = new_move(player->llHand->entry, origin[0], origin[1],
                last->ttl - 1);
    } else {
        int next[2] = {last->move[0], last->move[1]};
        if(next[0] + 1 == board->dim[0]) {
            // wrap to next line
            next[0] = 0;
            next[1]++;
        } else {
            next[0]++;
        }
        move = new_move(player->llHand->entry, next[0], next[1],
                last->ttl - 1);
    }
    return move;
}

/* Generate next attempt for auto P2;
 *
 * Player 2 acts in a similar way but starts at the bottom right corner and
 * starts at the right hand side of each row (right to left). The player will
 * move up a row if it doesn’t find a suitable space.
 */
Move* b2_move(Player* player, Board* board, Move* last) {
    int origin[2] = {board->dim[0], board->dim[1]};
    Move* move;
    if((last->ttl - 1) == (board->dim[0] * board->dim[1])) {
        move = new_move(player->llHand->entry, origin[0], origin[1],
                last->ttl - 1);
    } else {
        int next[2] = {last->move[0], last->move[1]};
        if(next[0] - 1 < 0) {
            // wrap to next line
            next[0] = board->dim[0];
            next[1]--;
        } else {
            next[0]--;
        }
        move = new_move(player->llHand->entry, next[0], next[1],
                last->ttl - 1);
    }
    return move;
}

/* Builds a Move struct to house data */
Move* new_move(char* card, int x, int y, int ttl) {
    Move* attempt = (Move*)malloc(sizeof(Move));
    attempt->ttl = ttl;
    attempt->card = (char*)malloc(sizeof(char) * CARD_SIZE);
    memcpy(attempt->card, card, sizeof(char) * CARD_SIZE);
    attempt->move = (int*)malloc(sizeof(int) * 2);
    attempt->move[0] = x;
    attempt->move[1] = y;
    return attempt;
}

/* Removes the indexed card from the hand, and shuffles remaining to lower
 * index.
 */
Player* remove_card(Player* player, char* card) {
    player->llHand = lstdel(player->llHand, card);
    return player;
}

/* Adds a new card to the lowest empty index (NULL represented) */
Player* add_card(Player* player, char* card) {
    if(strcmp(player->llHand->entry, EMPTY_STRING) == 0) {
        memcpy(player->llHand->entry, card, sizeof(char) * CARD_SIZE);
    } else {
        lstapp(player->llHand, card);
    }
    return player;
}
