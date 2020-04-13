/* Libraries */

#include "game.h"

/* Local Static Definitions */

/* Local Data Types */

/* Local Prototypes */

void msg_all(struct Client* players, char* message, bool toOrigin);
struct Move* read_move(struct Client* player);

/* Functions */

/* Checks the input string is a valid card and returns true if so */
bool is_card(char* rawCard) {
    // either SR or SR\n
    if(!(strlen(rawCard) == 2 || strlen(rawCard) == 3) ||
            !isalpha(rawCard[0])) {
        return false;
    }
    // all cards of style "{Suit}{Hex Value}\0"
    if(isdigit(rawCard[1])) {
        return true;
    } else {
        // maybe an alpha digit
        for(int i = 0; i < (int)strlen(ALPHA_DIGITS); i++) {
            if(rawCard[1] == ALPHA_DIGITS[i]) {
                return true;
            }
        }
        // wrong case || not one
        return false;
    }
}

/* Builds a new cards struct from a raw string input */
Card new_card(char* rawCard) {
    Card newCard;
    newCard.suit = rawCard[0];
    newCard.rank = strtol(&rawCard[1], NULL, 16);
    newCard.played = false;
    return newCard;
}

/* Checks to ensure the player actually has the card they say */
bool has_card(struct Client* player, char* rawCard) {
    for(int i = 0; i < player->handSize; i++) {
        if(rawCard[0] == player->hand[i].suit &&
                rawCard[1] == player->hand[i].rank &&
                !player->hand[i].played) {
            return true;
        }
    }
    // didn't find it
    return false;
}

/* Prints the lead player to stdout */
void print_lead(int leadID) {
    printf(OUT_LEAD, leadID);
    fflush(stdout);
}

/* Prints the cards played in this round */
void print_cards(Card* cards, int nPlayers) {
    printf(OUT_CARDS);
    for(int i = 0; i < nPlayers; i++) {
        printf(OUT_CARDS_PLACED, cards[i].suit, (int)cards[i].rank);
    }
    // remove extra space
    printf("%c\n", BACKSPACE);
    fflush(stdout);
}

/* Prints the current scores */
void print_scores(int* scores, int nPlayers) {
    for(int i = 0; i < nPlayers; i++) {
        printf(OUT_SCORES, i, scores[i]);
    }
    // remove that extra space
    printf("%c\n", BACKSPACE);
    fflush(stdout);
}

/* Prints the round of cards played to stderr from the player with lead */
void print_round(int leadID, struct LList* cardsPlayed) {
    fprintf(stderr, ERR_LEAD, leadID);
    for(struct LList* it = cardsPlayed; it != NULL; it = it->next) {
        fprintf(stderr, ERR_CARDS, it->entry[0], it->entry[1]);
    }
    fprintf(stderr, "%c\n", BACKSPACE);
    fflush(stderr);
}

/* Sends a message to all players if(toOrigin), else to all others */
void msg_all(struct Client* players, char* message, bool toOrigin) {
    // printf("TODO: remove debug counter\n");
    int debug = 0, debugCap = 5;
    int leadID = players->playerID;
    // print to source
    if(toOrigin) {
        write_pipe(players->player, message);
    }
    // print to rest
    players = players->next;
    while(players->playerID != leadID && debug < debugCap) {
        debug++;
        // write current
        write_pipe(players->player, message);
        // get next
        players = players->next;
    }
    if(debug == debugCap) {
        printf("DEBUG: msg_all exceeded loop limit (%d)\n", debug);
    }
}

/* Reads the player's move from client pipe */
struct Move* read_move(struct Client* player) {
    // char suit;
    // int rank;
    struct Move instance;
    char buffer[STRING_BUFFER] = EMPTY_STRING;
    // int res = fscanf(player->player->read, MSG_PLAY, suit, rank);
    fgets(buffer, STRING_BUFFER, player->player->read);
    if(!starts_with(buffer, MSG_PLAY) || !is_card(buffer + strlen(MSG_PLAY))) {
        return NULL;
    } else {
        // good format
        instance.playerNo = player->playerID;
        instance.cardIndex = -1;
        instance.card = new_card(buffer + strlen(MSG_PLAY));
    }
    // return data
    struct Move* move = malloc(sizeof(struct Move));
    memcpy(move, &instance, sizeof(struct Move));
    return move;
}

/* Starts a new round from the hub with the lead player's index: returns the
 * hub for re-use.
 */
Game* new_round(Game* game, int lead) {
    // get the right client to the front of the line
    game->round = NULL;
    while(game->players->playerID != lead) {
        game->players = game->players->next;
    }
    // start the round
    char buffer[STRING_BUFFER] = EMPTY_STRING;
    sprintf(buffer, MSG_ROUND, lead);
    msg_all(game->players, buffer, true);
    // loop through reading moves
    // printf("TODO: remove loop catch\n");
    int debug = 0;
    while(debug < 5) {
        debug++;
        // read their move
        struct Move* move = read_move(game->players);
        if(move == NULL) {
            fprintf(stderr, "Player EOF\n");
            exit(6);
        }
        if(game->round == NULL) {
            char buffer[STRING_BUFFER] = "\0";
            sprintf(buffer, MSG_HAND_CONCAT, move->card.suit,
                    (int)move->card.rank);
            game->round = llist(buffer + 1);
        } else {
            game->round = llist_app_card(game->round, move->card.suit,
                    move->card.rank);
        }
        // update players (not me)
        char buffer[STRING_BUFFER] = "\0";
        sprintf(buffer, MSG_PLAYED, lead, move->card.suit,
                (int)move->card.rank);
        msg_all(game->players, buffer, false);
        // next player
        game->players = game->players->next;
        if(game->players->playerID == lead) {
            // back to square one
            break;
        }
    }
    // // read scores & declare winner
    // top_score(game->round, lead, game->noPlayers);
    return game;
}

/* Takes the round's data and the lead index to determine the round winner */
int top_score(struct LList* round, int lead, int players) {
    char* leadCard = round->entry;
    int leadIndex = lead;
    int i = 0;
    // printf("DEBUG: lead suit %s\n", leadCard);
    for(struct LList* it = round->next; it != NULL; it = it->next) {
        if(it->entry[0] == leadCard[0] && it->entry[1] > leadCard[1]) {
            leadIndex = i % players;
        }
        i++;
    }
    // print out
    // Such a brilliantly evil idea that even I can't use this....
    printf(OUT_LEAD, leadIndex);
    printf(OUT_CARDS);
    for(struct LList* it = round; it != NULL; it = it->next) {
        // yeah I can fix it somewhere else, but I lack the will power
        if(strlen(it->entry) == 2) {
            printf("%c.%c ", it->entry[0], it->entry[1]);
        } else {
            printf("%s ", it->entry);
        }
    }
    printf("%c\n", BACKSPACE);
    return leadIndex;
}

/* Kills all players */
void kill_players(struct Client* players) {
    while(players->alive) {
        // kill the player
        kill(players->player->pid, SIGTERM);
        wait(NULL);
        players->alive = false;
        // to next
        players = players->next;
    }
}

/* Terminates the game in case of SIGHUP, terminating all Players */
void game_kill(Game* game, char* output) {
    // printf("TODO: kill all players && free junk\n");

    // kill_players(game->players);

    // print to stderr and return exit code
    fprintf(stderr, "%s", output);
    game != NULL ? exit(9) : exit(9);
}

/* Terminates the game with a proper GAMEOVER status at the end of deck */
void game_over(Game* game) {
    // printf("TODO: finish game & print scores\n");
    if(game) {
        exit(0);
    };
}
