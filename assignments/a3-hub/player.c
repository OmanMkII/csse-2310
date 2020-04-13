/* Libraries */

#include "player.h"

/* Local Prototypes */

// handle inputs
GameState* read_input(Player* player, char* rawInput);

struct Hand* read_player_hand(char* rawHand);
struct Move* read_other_move(char* rawMove);

bool valid_player(Player* player, char* rawPlayer);
bool valid_move(Player* player, char* rawMove);

// handle IO
Player* process_input(GameState* state, Player* player,
        void (*move)(Player*, bool, char));

Player* setup_hand(union Input newHand, Player* player);
Player* init_new_round(union Input round, Player* player,
        void (*move)(Player*, bool, char));
Player* update_last_move(union Input lastPlay, Player* player,
        void (*move)(Player*, bool, char));
// void game_over(GameState* state, Player* player);

// Free all data
void quit(GameState* state, Player* player, int exitcode);

/* Functions */

/* Loads all data in for a fresh game state */
Player* new_player(int numArgs, char** input) {
    Player instance;
    instance.gameover = false;
    if(numArgs != CLIENT_ARGNUM) {
        quit(NULL, NULL, ERRNO_ARGNUM);
    } else {
        // check n(players)
        if(!is_num(input[1]) || atoi(input[1]) < MIN_PLAYERS) {
            quit(NULL, NULL, ERRNO_PLAYERS);
        }
        instance.players = atoi(input[1]);
        // check my id
        if(!is_num(input[2]) || atoi(input[2]) < 0 ||
                atoi(input[2]) >= instance.players) {
            quit(NULL, NULL, ERRNO_POSITION);
        }
        instance.position = atoi(input[2]);
        // check threshold
        if(!is_num(input[3]) || atoi(input[3]) < MIN_THRESHOLD) {
            quit(NULL, NULL, ERRNO_THRESHOLD);
        }
        instance.threshold = atoi(input[3]);
        // check hand size
        if(!is_num(input[4]) || atoi(input[4]) < 1) {
            quit(NULL, NULL, ERRNO_HANDSIZE);
        }
        instance.hand.size = atoi(input[4]);
        instance.hand.cards = NULL;
        instance.round.cardsPlayed = NULL;
        instance.dScores.count = NULL;
    }
    // print symbol and return
    Player* newPlayer = malloc(sizeof(Player));
    memcpy(newPlayer, &instance, sizeof(Player));
    printf(MSG_SETUPCOMPLETE);
    fflush(stdout);
    return newPlayer;
}

/* Reads the player's hand from stdin and returns a Hand struct for use */
struct Hand* read_player_hand(char* rawHand) {
    struct Hand instance;
    char** handSplit = split_string(rawHand, COMMA, true);
    // n of cards
    if(!is_num(handSplit[0]) ||
            array_sizeof(handSplit) - 1 != atoi(handSplit[0])) {
        free_array(handSplit);
        return NULL;
    }
    instance.size = atoi(handSplit[0]);
    // Add each card
    int i = 1;
    Card* cards = malloc(sizeof(Card) * instance.size);
    while(handSplit[i] != NULL) {
        if(is_card(handSplit[i])) {
            cards[i - 1] = new_card(handSplit[i]);
        } else {
            fflush(stdout);
            free(cards);
            return NULL;
        }
        i++;
    }
    instance.cards = cards;
    // free stuff
    free_array(handSplit);
    // return struct
    struct Hand* hand = malloc(sizeof(struct Hand));
    memcpy(hand, &instance, sizeof(struct Hand));
    return hand;
}

/* Reads another player's Move from stdin and returns a struct for use */
struct Move* read_other_move(char* rawMove) {
    struct Move instance;
    instance.cardIndex = -1;
    char* temp = malloc(sizeof(char) * STRING_BUFFER);
    memcpy(temp, rawMove, sizeof(char) * STRING_BUFFER);
    char** move = split_string(temp, COMMA, true);
    // check length
    if(array_sizeof(move) != 2) {
        free_array(move);
        return NULL;
    }
    // check it's valid
    if(!is_num(move[0]) || atoi(move[0]) < 0) {
        free_array(move);
        return NULL;
    }
    instance.playerNo = atoi(move[0]);
    // check card is valid
    if(!is_card(move[1])) {
        free_array(move);
        return NULL;
    }
    instance.card = new_card(move[1]);
    // free
    free_array(move);
    free(temp);
    // return move
    struct Move* newMove = malloc(sizeof(struct Move));
    memcpy(newMove, &instance, sizeof(struct Move));
    return newMove;
}

/* Checks that the input given fits the bounds already known to the player */
bool valid_player(Player* player, char* rawPlayer) {
    char playerNo[STRING_BUFFER] = "\0";
    int i = 0;
    while(rawPlayer[i] != '\n' && rawPlayer[i] != '\0') {
        playerNo[i] = rawPlayer[i];
        i++;
    }
    playerNo[i] = '\0';
    return (is_num(playerNo) &&
            (0 <= atoi(playerNo) && atoi(playerNo) < player->players));
}

/* Checks that the move is valid to the player's known bounds */
bool valid_move(Player* player, char* rawMove) {
    char* rawCopy = malloc(sizeof(char) * (strlen(rawMove) + 1));
    memcpy(rawCopy, rawMove, sizeof(char) * (strlen(rawMove) + 1));
    char** move = split_string(rawCopy + strlen(COMM_PLAYER), COMMA, true);
    if(valid_player(player, move[0]) &&
            isalpha(move[1][0]) && isxdigit(move[1][1])) {
        bool state = (strchr(SUITS, move[1][0]) != NULL);
        free_array(move);
        free(rawCopy);
        return state;
    } else {
        return false;
    }
}

/* Reads raw char input and translates to GameState data type for processing,
 * and takes the player struct to use for arg checking.
 */
GameState* read_input(Player* player, char* rawInput) {
    GameState instance;
    if(starts_with(rawInput, COMM_HAND)) {
        // HANDn,SR,SR,SR
        instance.inputType = COMM_HAND_NO;
        char* input = rawInput + strlen(COMM_HAND);
        struct Hand* hand = read_player_hand(input);
        if(hand != NULL) {
            instance.input.playerHand = *hand;
            free(hand);
        } else {
            return NULL;
        }
    } else if(starts_with(rawInput, COMM_NEW_ROUND) &&
            valid_player(player, rawInput + strlen(COMM_NEW_ROUND))) {
        // NEWROUNDL
        instance.inputType = COMM_NEW_ROUND_NO;
        instance.input.leadPlayer = atoi(rawInput + strlen(COMM_NEW_ROUND));
    } else if(starts_with(rawInput, COMM_PLAYER) &&
            valid_move(player, rawInput)) {
        // PLAYEDw,SR
        instance.inputType = COMM_PLAYER_NO;
        char* input = rawInput + strlen(COMM_PLAYER);
        struct Move* other = read_other_move(input);
        instance.input.otherMove = *other;
        free(other);
    } else if(starts_with(rawInput, COMM_GAMEOVER) &&
            strlen(rawInput) == strlen(COMM_GAMEOVER)) {
        // GAMEOVER
        instance.inputType = COMM_GAMEOVER_NO;
        instance.input.gameOver = true;
    } else {
        // invalid incoming, handled later
        return NULL;
    }
    // instantiate and return
    GameState* move = malloc(sizeof(GameState));
    memcpy(move, &instance, sizeof(GameState));
    return move;
}

/* Takes hand information from the hub, then sets the player's hand and returns
 * the updated struct
 */
Player* setup_hand(union Input newHand, Player* player) {
    player->hand = newHand.playerHand;
    return player;
}

/* Prints the cards dealt from the last round and the winner to stderr */
void print_last_round(int winner, struct LList* cardsPlayed) {
    fprintf(stderr, ERR_LEAD, winner);
    print_llist(cardsPlayed, SPACE);
    fprintf(stderr, NEW_LINE);
}

/* Takes the number of the lead player to store for later flagging, updates the
 * player's flags to true is this player is starting the round and adds an
 * index as well: if they are the lead player they will make their move.
 */
Player* init_new_round(union Input round, Player* player,
        void (*move)(Player*, bool, char)) {
    // if round completed, print data
    if(player->round.cardsPlayed != NULL) {
        // print_last_round(player->round.lead, player->round.cardsPlayed);
        llist_clear(player->round.cardsPlayed);
        free(player->round.cardsPlayed);
        player->round.cardsPlayed = NULL;
    }
    // make new round
    player->round.lead = round.leadPlayer;
    // if lead, make play
    if(player->round.lead == player->position) {
        (move)(player, true, player->round.leadSuit);
        player->round.noPlays++;
    }
    return player;
}

/* Takes information of the last move, and updates the player if they are the
 * lead: if they are the next player they will make their own play.
 */
Player* update_last_move(union Input lastPlay, Player* player,
        void (*move)(Player*, bool, char)) {
    // set lead suit
    if(lastPlay.otherMove.playerNo == player->round.lead) {
        player->round.leadSuit = lastPlay.otherMove.card.suit;
    }
    // append to played cards
    char* play = malloc(sizeof(char) * STRING_BUFFER);
    sprintf(play, ERR_CARDS, lastPlay.otherMove.card.suit,
            (int)lastPlay.otherMove.card.rank);
    if(player->round.cardsPlayed == NULL) {
        player->round.cardsPlayed = llist(play);
    } else {
        player->round.cardsPlayed = llist_app(player->round.cardsPlayed, play);
    }
    player->round.noPlays++;
    free(play);
    // check if next play
    if(!player->round.havePlayed) {
        // if next, make move
        if((lastPlay.otherMove.playerNo + 1) % player->players ==
                player->position) {
            (move)(player, false, player->round.leadSuit);
            player->round.noPlays++;
        }
    }
    // else not relevant to me
    return player;
}

/* A hub that takes the incoming state of the game to update or destroy the
 * player struct as necessary: also takes a pointer to the player's move
 * function for when it's their turn.
 */
Player* process_input(GameState* state, Player* player,
        void (*move)(Player*, bool, char)) {
    // function pointer setup
    Player* updatedPlayer = NULL;
    // initial setup
    Player* (*setup)(union Input, Player*) = setup_hand;
    // start of round && during round
    Player* (*round[])(union Input, Player*,
            void (*move)(Player*, bool, char)) = {init_new_round,
            update_last_move};
    // end of game
    // void (*gameover)(GameState*, Player*) = game_over;
    // find correct function
    switch(state->inputType) {
        case COMM_HAND_NO:
            updatedPlayer = setup(state->input, player);
            break;

        case COMM_NEW_ROUND_NO:
        case COMM_PLAYER_NO:
            updatedPlayer = round[state->inputType - COMM_OFFSET](state->input,
                    player, move);
            // if end of round, print scores
            if(player->round.noPlays == player->players) {
                print_last_round(player->round.lead,
                        player->round.cardsPlayed);
            }
            break;

        case COMM_GAMEOVER_NO:
            quit(state, player, 0);
            break;

        default:
            printf("DEBUG: bad input state\n");
            fflush(stdout);
    }
    // ensure return
    free(state);
    return updatedPlayer;
}

/* Takes the player's hand and returns a struct containing the index of highest
 * and lowest cards to assist plays.
 */
struct Query* query_hand(struct Hand hand, char suit) {
    struct Query instance;
    instance.highest = -1;
    int valHighest = 0;
    instance.lowest = -1;
    int valLowest = 0;
    for(int i = 0; i < hand.size; i++) {
        if(hand.cards[i].played) {
            continue;
        } else if(hand.cards[i].suit == suit) {
            if(hand.cards[i].rank < valLowest ||
                    instance.lowest == -1) {
                instance.lowest = i;
                valLowest = hand.cards[i].rank;
            }
            if(hand.cards[i].rank > valHighest ||
                    instance.highest == -1) {
                instance.highest = i;
                valHighest = hand.cards[i].rank;
            }
        }
    }
    // return data || NULL
    if(instance.highest == -1 || instance.lowest == -1) {
        return NULL;
    } else {
        struct Query* hand = malloc(sizeof(struct Query));
        memcpy(hand, &instance, sizeof(struct Query));
        return hand;
    }
}

/* Plays the card given by printing the required output to stdout */
Player* play_card(Player* player, int cardno) {
    Card card = player->hand.cards[cardno];
    // play card
    printf(COMM_MAKE_PLAY, card.suit, (int)card.rank);
    fflush(stdout);
    // store play
    if(player->round.cardsPlayed == NULL) {
        player->round.cardsPlayed = llist(EMPTY_STRING);
    }
    player->round.cardsPlayed = llist_app_card(player->round.cardsPlayed,
            card.suit, card.rank);
    // return
    player->hand.cards[cardno].played = true;
    return player;
}

/* Manages how the client interacts with the game: takes the player struct as
 * main data and a pointer to the player's personal strategy for implementing.
 */
Player* make_play(Player* player, void (*move)(Player*, bool, char),
        bool newGame) {
    char input[STRING_BUFFER] = EMPTY_STRING;
    // read stdin when modified
    if(fgets(input, STRING_BUFFER, stdin) == NULL) {
        quit(NULL, player, ERRNO_EOF);
    }
    // fprintf(stderr, "DEBUG (%d): input \"%s\"\n", getpid(), input);
    // interpret
    GameState* state = read_input(player, input);
    if(state == NULL) {
        // bad input from hub
        quit(NULL, player, ERRNO_MESSAGE);
    } else {
        // ensure we got it in the right order
        if((!newGame && state->inputType == COMM_HAND_NO) ||
                (newGame && state->inputType != COMM_HAND_NO)) {
            // given hand again || anything else before hand
            quit(state, player, ERRNO_MESSAGE);
        } else {
            newGame = false;
        }
        // process input
        player = process_input(state, player, move);
    }
    return player;
}

/* Exit the player's game with a given exit code and matching error output */
void quit(GameState* state, Player* player, int exitcode) {
    if(state != NULL) {
        free(state);
    }
    if(player != NULL) {
        if(player->hand.cards != NULL) {
            free(player->hand.cards);
        }
        if(player->round.cardsPlayed != NULL) {
            llist_clear(player->round.cardsPlayed);
            free(player->round.cardsPlayed);
        }
        free(player);
    }
    // print error
    switch(exitcode) {
        case ERRNO_ARGNUM:
            fprintf(stderr, ERR_ARGNUM);
            exit(1);

        case ERRNO_PLAYERS:
            fprintf(stderr, ERR_PLAYERS);
            exit(2);

        case ERRNO_POSITION:
            fprintf(stderr, ERR_POSITION);
            exit(3);

        case ERRNO_THRESHOLD:
            fprintf(stderr, ERR_THRESHOLD);
            exit(4);

        case ERRNO_HANDSIZE:
            fprintf(stderr, ERR_HANDSIZE);
            exit(5);

        case ERRNO_MESSAGE:
            fprintf(stderr, ERR_MESSAGE);
            exit(6);

        case ERRNO_EOF:
            fprintf(stderr, ERR_EOF);
            exit(7);

        default:
            exit(0);
    }
}
