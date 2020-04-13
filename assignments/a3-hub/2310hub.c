/* Libraries */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "game.h"
#include "player.h"

/* Global Variables */

// The state of the game: used to flag for termination with SIGHUP
bool* recvSighup;

/* Static Definitions */

// Errors
#define ERR_HUBARGS "Usage: 2310hub deck threshold player0 {player1}\n"
#define ERR_BADTHRESH "Invalid threshold\n"
#define ERR_BADDECK "Deck error\n"
#define ERR_CARDNUM "Not enough cards\n"
#define ERR_RECV_ERR "Player error\n"
#define ERR_RECV_EOF "Player EOF\n"
#define ERR_BADMSG "Invalid message\n"
#define ERR_BADCHOICE "Invalid card choice\n"
#define ERR_SIGHUP "Ended due to signal\n"

// general
#define CLIENT_OFFSET 3
#define DEFAULT 0

/* Data Types */

/* Local Prototypes */

// utility
char** client_args(int* values, int numArgs);
void handle_sighup(int signum);

// game building
struct LList* read_deck(char* filepath);
Game* new_game(int numArgs, char** input);
struct Client* launch_clients(char** clientList, int numClients, int index,
        char** input);

/* Main */

int main(int argc, char** argv) {
    // One fine day, in the middle of the night
    recvSighup = malloc(sizeof(bool));
    recvSighup[0] = false;
    signal(SIGHUP, handle_sighup);
    // Two dead men got up to fight
    Game* hub = new_game(argc, argv);
    int lead = DEFAULT;
    while(!recvSighup[0] && hub->roundNo != hub->cards) {
        // Back to back they faced each other
        hub = new_round(hub, lead);
        lead = top_score(hub->round, lead, hub->noPlayers);
    }
    // Drew their swords, and shot each other
    if(recvSighup[0]) {
        game_kill(hub, ERR_SIGHUP);
    } else {
        game_over(hub);
    }
    return 0;
}

/* Local Functions */

/* Reads the full deckfile and returns a list of cards if valid, else NULL */
struct LList* read_deck(char* filepath) {
    // Read deck file
    struct LList* deckfile = read_file(filepath);
    if(deckfile == NULL) {
        return NULL;
    }
    // Check cards && size valid
    int line = 0, size;
    // int size;
    for(struct LList* it = deckfile; it != NULL; it = it->next) {
        if(line == 0) {
            // set deck size
            if(!is_num(it->entry) || atoi(it->entry) < 0) {
                llist_clear(deckfile);
                return NULL;
            }
            size = atoi(it->entry);
        } else {
            if(!is_card(it->entry)) {
                llist_clear(deckfile);
                return NULL;
            }
        }
        line++;
    }
    // check valid length
    if(size != (line - 1)) {
        llist_clear(deckfile);
        return NULL;
    }
    // skip deck size
    struct LList* deck = deckfile->next;
    // free first entry
    free(deckfile->entry);
    free(deckfile);
    return deck;
}

/* Builds a new game from the input to the hub invocation */
Game* new_game(int numArgs, char** input) {
    Game instance;
    if(numArgs < MIN_ARGS) {
        fprintf(stderr, ERR_HUBARGS);
        exit(1);
    } else {
        // check threshold
        if(!is_num(input[2]) || atoi(input[2]) < 2) {
            fprintf(stderr, ERR_BADTHRESH);
            exit(2);
        }
        instance.threshold = atoi(input[2]);
        // check deck
        struct LList* deck = read_deck(input[1]);
        if(deck == NULL) {
            fprintf(stderr, ERR_BADDECK);
            exit(3);
        } else if(llist_sizeof(deck) < (numArgs - 3)) {
            // (n(args) - 3) == n(players)
            fprintf(stderr, ERR_CARDNUM);
            llist_clear(deck);
            exit(4);
        }
        instance.deck = deck;
        instance.deckIndex = 0;
        instance.cards = llist_sizeof(instance.deck);
        instance.round = NULL;
        instance.roundNo = 0;
        // check all players
        char** clients = &input[3];
        instance.noPlayers = numArgs - CLIENT_OFFSET;
        int handSize = floor(llist_sizeof(instance.deck) / instance.noPlayers);
        int playerInput[] = {instance.noPlayers, instance.threshold, handSize};
        instance.players = launch_clients(clients, instance.noPlayers, 0,
                client_args(playerInput, CLIENT_ARGNUM - 2));
        // printf("TODO: free args\n");
        if(instance.players == NULL) {
            fprintf(stderr, ERR_RECV_ERR);
            // printf("TODO: clean up players\n");
            exit(5);
        } else {
            // sort out hands
            struct LList* index = instance.deck;
            char** hands = malloc(sizeof(char*) * instance.noPlayers);
            for(int i = 0; i < instance.noPlayers; i++) {
                char hand[STRING_BUFFER] = "\0";
                sprintf(hand, MSG_HAND, handSize);
                for(int j = 0; j < handSize; j++) {
                    strcat(hand, ",");
                    strcat(hand, index->entry);
                    index = index->next;
                }
                strcat(hand, NEW_LINE);
                hands[i] = malloc(sizeof(char) * STRING_BUFFER);
                memcpy(hands[i], hand, sizeof(char) * STRING_BUFFER);
            }
            // circular logic makes the world go 'round
            int i = 0;
            struct Client* it = instance.players;
            struct Client* last;
            while(it != NULL) {
                write_pipe(it->player, hands[i]);
                free(hands[i]);
                i++;
                if(it->next == NULL) {
                    last = it;
                }
                it = it->next;
            }
            last->next = instance.players;
            free(hands);
        }
    }
    // return game instance
    Game* game = malloc(sizeof(Game));
    memcpy(game, &instance, sizeof(Game));
    return game;
}

/* Converts the integer based arguments into a char array for use */
char** client_args(int* values, int numArgs) {
    // format { n(players), threshold, handsize }
    char** args = malloc(sizeof(char*) * numArgs);
    for(int i = 0; i < numArgs; i++) {
        args[i] = malloc(sizeof(char) * STRING_BUFFER);
        sprintf(args[i], "%d", values[i]);
    }
    return args;
}

/* Launches all clients in the main call recursively and attaches them to a
 * Linked List CDT for a bit of fun. Each iteration takes the original list and
 * its size, as well as an incrememnting index to keep track of the position;
 * also requires constants of number of players, threshold, and hand size.
 */
struct Client* launch_clients(char** clientList, int numClients, int index,
        char** input) {
    printf("DEBUG: %dth client\n", index);
    struct Client instance;
    // attempt to launch
    char chardex[STRING_BUFFER] = "\0";
    sprintf(chardex, "%d", index);
    char* args[] = {clientList[index], input[0], chardex, input[1],
            input[2], NULL};
    // printf("DEBUG: setting up client %d (%s)\n", index, clientList[index]);
    PipeClient* client = pipe_prog(args, false);
    // printf("DEBUG: reading client's input\n");
    // char msg = read_pipe(client, "%c");
    char msg = fgetc(client->read);
    // printf("DEBUG: ensuring proper form\n");
    if(client == NULL || msg != MSG_SETUPCOMPLETE[0]) {
        // printf("DEBUG: bad client or null pipe\n");
        return NULL;
    // } else if(strcmp(msg, MSG_SETUPCOMPLETE) != 0) {
    //     printf("DEBUG: bad detup message\n");
    //     printf("TODO: free clients\n");
    //     return NULL;
    } else {
        instance.player = client;
    }
    // complete player
    instance.hand = NULL;
    instance.playerID = index;
    instance.handSize = atoi(input[2]);
    // iterate
    if(index == numClients - 1) {
        // end of list
        // printf("DEBUG: end of list\n");
        instance.next = NULL;
    } else {
        // Never ever ever forget to increment a forking recursion.
        instance.next = launch_clients(clientList, numClients, ++index, input);
        // check child worked [(N - 1)th index - 1]
        if(instance.next == NULL && index != numClients - 2) {
            // kill child (you monster!)
            kill_pipe(client);
            return NULL;
        }
    }
    // return this instance
    struct Client* player = malloc(sizeof(struct Client));
    memcpy(player, &instance, sizeof(struct Client));
    return player;
}

/* Deal with a SIGHUP receive */
void handle_sighup(int signum) {
    // flag gameover for termination
    recvSighup[0] = (signum == SIGHUP);
}
