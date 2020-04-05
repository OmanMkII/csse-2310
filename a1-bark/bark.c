/* Bark:
 *
 * The game consists of a deck of cards and a board (a rectangular grid with
 * spaces to place cards). The deck consists of cards each having a number
 * between 1 and 9 (inclusive) and a suit (represented by a capital letter).
 *
 * A new game will end immediately if there are not at least 11 cards in the
 * deck.
 *
 * Each player draws a starting hand of 5 cards from the deck. At the
 * beginning of each player’s turn, they draw one card from the deck and then
 * play one card onto the board in an unoccupied space. Each card played
 * (except for the first one) must be played immediately adjacenct
 * (horizontally or vertically) to a card already on the board. The game ends
 * at the end of the turn when either;
 *
 *  • the last card has been drawn from the deck.
 *  • there is no free space on the board.
 */

/* Libraries */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "deck.h"
#include "exit.h"
#include "score.h"
#include "player.h"
#include "2310util.h"

/* Static Def'ns */

#define ARGC_LOAD 4
#define ARGC_NEW 6

#define DIM_MIN 3
#define DIM_MAX 100
#define MIN_CARDS 11
#define START_HAND 5
#define ACTIVE_HAND 6

#define PLAYER_1 1
#define PLAYER_2 2
#define PLAYER_OFFSET 2
#define PLAYER_LINE_OFFSET_A 1
#define PLAYER_LINE_OFFSET_B 2

#define SCORES "Player 1=%d Player 2=%d\n"
#define UNASSIGNED 'x'
#define EMPTY '*'
#define EMPTY_TILE ".."
#define EMPTY_WRITE "**"

#define PRINT_DECK true
#define SAVE_DECK false
#define ALPHABET_SIZE 26
#define ODD "ACEGIKMOQSUWY"
#define EVEN "BDFHJLNPRTVXZ"

#define LINE_METADATA "%d %d %d %d\n"
#define LINE_DECKFILE "%s\n"
#define LINE_PLAYER_N "%s"
#define LINE_BOARD_N "%s\n"

/* Data Types */

struct SaveLoader {
    Board* board;
    int index;
    int turn;
};

/* Prototypes */

// init game
Bark* make_game(int nArgs, char** input);
Bark* new_game(char** input);
Bark* load_game(char** input);
Bark* read_save(char* filepath, Bark* game);

// save loading helpers
struct SaveLoader* read_meta_data(char* line);
Player* import_player(Player* player, char* hand, bool turn);
Board* rebuild_game(Board* newBoard, char* line, int lineNo);

// save game
void save_game(Bark* game, int turn, char* filepath);

// Game board
Board* new_board(char* width, char* height);
struct LList* print_board(Board* board, bool print);

// running the game
Bark* draw_card(Bark* game, int player, int num);
Move* prompt_player(Bark* game, int turn, bool isFirst);
Board* place_card(Board* board, Move* move, int pNo, bool isBot);

// Check for valid plays
bool has_space(Board* board);
bool is_valid(Board* board, Move* attempt, bool first);
bool slot_is_full(Board* board, int* source, int* delta);

/* Main */

int main(int argc, char** argv) {
    // // TODO: remove
    // for(int i = 0; i < argc; i++) {
    //     printf("%d: %s\n", i, argv[i]);
    // }
    // Load game
    Bark* game = make_game(argc, argv);
    // Have some fun
    bool full = false;
    // int turn = game->; // nth player + 1
    print_board(game->board, PRINT_DECK);
    while(game->deckIndex != NULL && (full = has_space(game->board))) {
        // draw cards
        if(lstget(game->players[game->turn - 1]->llHand, 5) == NULL) {
            game = draw_card(game, game->turn, 1);
        }
        // prompt either player
        Move* move = prompt_player(game, game->turn, game->first);
        game->first = false;
        game->players[game->turn - 1]->llHand =
                lstdel(game->players[game->turn - 1]->llHand, move->card);
        game->board = place_card(game->board, move, game->turn,
                (game->players[game->turn - 1]->type == AUTO));
        // Set next turn
        if(game->turn == PLAYER_1) {
            game->turn = PLAYER_2;
        } else {
            game->turn = PLAYER_1;
        }
        print_board(game->board, PRINT_DECK);
    }
    // print scores || save game
    int* scores = calc_scores(game->board);
    printf(SCORES, scores[0], scores[1]);
    return 0;
}

/* Functions */

/* Builds the game from command line inputs */
Bark* make_game(int nArgs, char** input) {
    Bark* game = (Bark*)malloc(sizeof(Bark));
    // Load the game
    if(nArgs == ARGC_LOAD) {
        game = load_game(input);
        game->first = false;
    } else if(nArgs == ARGC_NEW) {
        game = new_game(input);
        if(get_deck_size(game->deck) < 11) {
            exit_prog(SHORT_DECK, game);
        } else {
            game = draw_card(game, PLAYER_1, 5);
            game = draw_card(game, PLAYER_2, 5);
        }
        game->turn = PLAYER_1;
        game->first = true;
    } else {
        exit_prog(ARGS_NUM, game);
    }
    return game;
}

/* Builds a fresh game from the given args */
Bark* new_game(char** input) {
    // usage: bark deckfile width height p1type p2type
    Bark* game = (Bark*)malloc(sizeof(Bark));
    // Set player types
    game->players = (Player**)malloc(sizeof(Player*) * 2);
    game->players[0] = new_player(input[4]);
    if(game->players[0] == NULL) {
        exit_prog(ARGS_TYPE, game);
    }
    game->players[1] = new_player(input[5]);
    if(game->players[1] == NULL) {
        exit_prog(ARGS_TYPE, game);
    }
    // New board
    Board* newBoard = new_board(input[2], input[3]);
    if(newBoard != NULL) {
        game->board = newBoard;
    } else {
        free(newBoard);
        exit_prog(2, game);
    }
    // Read deck file
    struct DeckHolder* deckHolder = read_deck(input[1], 0);
    if(deckHolder == NULL) {
        exit_prog(FILE_DECK, game);
    } else {
        game->deckfile = input[1];
        game->deck = deckHolder->deckList;
        game->deckIndex = deckHolder->deckIndex;
        free(deckHolder);
    }
    return game;
}

/* Loads a game from the given save file */
Bark* load_game(char** input) {
    // usage: bark savefile p1type p2type
    Bark* game = (Bark*)malloc(sizeof(Bark));
    // Set player types
    game->players = (Player**)malloc(sizeof(Player*) * 2);
    Player* p1 = new_player(input[2]);
    if(p1 == NULL) {
        exit_prog(ARGS_TYPE, game);
    } else {
        game->players[0] = p1;
    }
    Player* p2 = new_player(input[3]);
    if(p2 == NULL) {
        exit_prog(ARGS_TYPE, game);
    } else {
        game->players[1] = p2;
    }
    // Read save file
    game = read_save(input[1], game);
    if(game == NULL) {
        // NOTE: bad deck > bad save --> exit (5) when reading
        exit_prog(FILE_SAVE, game);
    }
    return game;
}

/* Reads data given from the save file to rebuild the game */
Bark* read_save(char* filepath, Bark* game) {
    // Read save file
    struct LList* save = readfl(filepath);
    if(save == NULL) {
        exit_prog(FILE_SAVE, NULL);
    }
    // Iterate save file
    struct SaveLoader* savedata;
    int line = 0, height = 0;
    for(struct LList* it = save; it != NULL; it = it->next) {
        if(line == 0) {
            savedata = read_meta_data(it->entry);
            if(save == NULL) {
                exit_prog(FILE_SAVE, game);
            } else {
                game->board = savedata->board;
                game->turn = savedata->turn;
            }
        } else if(line == 1) {
            // read deckfile (bad deck > bad save)
            struct DeckHolder* deckHolder =
                    read_deck(it->entry, savedata->index);
            if(deckHolder == NULL) {
                exit_prog(FILE_DECK, game);
            } else {
                game->deck = deckHolder->deckList;
                game->deckfile = it->entry;
                game->deckIndex = deckHolder->deckIndex;
                free(deckHolder);
            }
        } else if(line == 2 || line == 3) {
            // read players
            Player* p = import_player(game->players[line - PLAYER_OFFSET],
                    it->entry, savedata->turn ==
                    (line - PLAYER_LINE_OFFSET_A));
            if(p == NULL) {
                exit_prog(FILE_SAVE, game);
            } else {
                game->players[line - PLAYER_LINE_OFFSET_B] = p;
            }
        } else {
            game->board = rebuild_game(game->board, it->entry, height++);
        }
        line++;
    }
    // check board height
    if(height != game->board->dim[1]) {
        exit_prog(FILE_SAVE, game);
    }
    // TODO
    return game;
}

/* Builds metadata from line 1 of save */
struct SaveLoader* read_meta_data(char* line) {
    // <width> <height> <n. drawn> <turn> [all digits]
    char** setup = spltstr(line, SPACE);
    // check parameters valid
    if(setup[4] != NULL) {
        return NULL;
    } else if(!isnum(setup[2]) || !isnum(setup[3])) {
        return NULL;
    }
    // data storage
    struct SaveLoader* save = malloc(sizeof(struct SaveLoader));
    // cards drawn (to assign a ptr later)
    save->index = atoi(setup[2]);
    if(save->index < 0) {
        return NULL;
    }
    // turn
    save->turn = atoi(setup[3]);
    if(save->turn != 1 && save->turn != 2) {
        return NULL;
    }
    // game dimensions
    Board* board = new_board(setup[0], setup[1]);
    if(board == NULL) {
        return NULL;
    } else {
        save->board = board;
    }
    return save;
}

/* Imports player from save file args */
Player* import_player(Player* player, char* hand, bool turn) {
    // Check valid
    if(turn && strlen(hand) != (ACTIVE_HAND * 2)) {
        return NULL;
    } else if(!turn && strlen(hand) != START_HAND * 2) {
        return NULL;
    }
    // build hand
    int handSize = (turn ? ACTIVE_HAND : START_HAND);
    struct LList* llHand = llist(EMPTY_STRING);
    for(int i = 0; i < handSize; i++) {
        // read cards
        if(!isdigit(hand[2 * i]) || !isalpha(hand[2 * i + 1])) {
            lstclr(llHand);
            free(llHand);
            return NULL;
        } else {
            char card[CARD_SIZE];
            memcpy(card, &hand[2 * i], sizeof(char) * CARD_SIZE);
            card[2] = END_STRING;
            llHand = lstapp(llHand, card);
        }
    }
    // assign to player
    player->llHand = llHand;
    return player;
}

/* Reconstructs a save game from the save file */
Board* rebuild_game(Board* newBoard, char* line, int lineNo) {
    // TODO: free partial deck
    if((int)strlen(line) != 2 * newBoard->dim[0]) {
        return NULL;
    } else {
        for(int i = 0; i < newBoard->dim[0]; i++) {
            if(line[2 * i] == EMPTY && line[2 * i + 1] == EMPTY) {
                newBoard->slots[i][lineNo] = NULL;
            } else if(isdigit(line[2 * i]) && isalpha(line[2 * i + 1])) {
                char card[3];
                sprintf(card, "%c%c", line[2 * i], line[2 * i + 1]);
                // newBoard->slots[lineNo][i] = new_card(card);
                newBoard->slots[i][lineNo] = new_card(card);
            } else {
                return NULL;
            }
        }
        return newBoard;
    }
}

/* Exports a save game file */
void save_game(Bark* game, int turn, char* filepath) {
    // 0: open file
    FILE* fl = fopen(filepath, "w+");
    // 1: <width> <height> <n drawn> <turn>
    int drawn = 0;
    struct LList* it = game->deck;
    while(strcmp(it->entry, game->deckIndex->entry)) {
        it = it->next;
        drawn++;
    }
    fprintf(fl, LINE_METADATA, game->board->dim[0], game->board->dim[1],
            drawn, turn);
    // 2: deckfile name
    fprintf(fl, LINE_DECKFILE, game->deckfile);
    // 3: player 1
    for(struct LList* it = game->players[0]->llHand;
            it != NULL; it = it->next) {
        fprintf(fl, LINE_PLAYER_N, it->entry);
    }
    fprintf(fl, NEW_LINE);
    // 4: player 2
    for(struct LList* it = game->players[1]->llHand;
            it != NULL; it = it->next) {
        fprintf(fl, LINE_PLAYER_N, it->entry);
    }
    fprintf(fl, NEW_LINE);
    // 5+: board
    struct LList* board = print_board(game->board, SAVE_DECK);
    for(struct LList* it = board; it != NULL; it = it->next) {
        fprintf(fl, LINE_BOARD_N, it->entry);
    }
    // TODO: if I don't want trailing \n, put it before #5, don't have on #4
    fclose(fl);
}

/* Generates a fresh board for new games */
Board* new_board(char* width, char* height) {
    // check vars
    if(!isnum(width) || !isnum(height)) {
        return NULL;
    }
    int w = atoi(width);
    if(w < DIM_MIN || DIM_MAX < w) {
        return NULL;
    }
    int h = atoi(height);
    if(h < DIM_MIN || DIM_MAX < h) {
        return NULL;
    }
    // Build new board
    Board* newBoard = (Board*)malloc(sizeof(Board));
    newBoard->dim = (int*)malloc(sizeof(int) * 2);
    newBoard->dim[0] = w;
    newBoard->dim[1] = h;
    newBoard->slots = malloc(sizeof(Card**) * newBoard->dim[0]);
    for(int k = 0; k < newBoard->dim[0]; k++) {
        newBoard->slots[k] = malloc(sizeof(Card*) * newBoard->dim[1]);
        // NULL entries == empty spaces
        for(int l = 0; l < newBoard->dim[1]; l++) {
            newBoard->slots[k][l] = NULL;
        }
    }
    return newBoard;
}

/* Prints the entire game board, to struct if enabled */
struct LList* print_board(Board* board, bool print) {
    struct LList* out;
    out = (print ? NULL : llist(NULL));
    for(int i = 0; i < board->dim[1]; i++) {
        char line[STRING_BUFFER];
        for(int j = 0; j < board->dim[0]; j++) {
            if(board->slots[j][i] == NULL) {
                print ? printf("%s", EMPTY_TILE) :
                        sprintf(line, "%s", EMPTY_WRITE);
            } else {
                print ? printf("%d%c", board->slots[j][i]->value,
                        board->slots[j][i]->suit) :
                        sprintf(line, "%d%c", board->slots[j][i]->value,
                        board->slots[j][i]->suit);
            }
        }
        print ? printf(NEW_LINE) : sprintf(line, NEW_LINE);
        out = (print ? NULL : lstapp(out, line));
    }
    return out;
}

/* Draws a new card for the selected player */
Bark* draw_card(Bark* game, int player, int num) {
    for(int i = 0; i < num; i++) {
        game->players[player - 1] = add_card(game->players[player - 1],
                game->deckIndex->entry);
        game->deckIndex = game->deckIndex->next;
    }
    return game;
}

/* Prompts the player for command input */
Move* prompt_player(Bark* game, int turn, bool isFirst) {
    Move* attempt = make_turn(game->players[turn - 1], game->board,
            NULL, turn);
    while(!is_valid(game->board, attempt, isFirst)) {
        // first = false;
        if(attempt == NULL) {
            // EOF from input
            exit_prog(EOINPUT, game);
        } else if(attempt->save) {
            save_game(game, turn, attempt->savepath);
        } else if(attempt->ttl == 0) {
            fprintf(stderr, "DEBUG: ttl expired\n");
            exit(8);
        }
        // printf("DEBUG: ttl = %d\n", attempt->ttl);
        attempt = make_turn(game->players[turn - 1], game->board,
                attempt, turn);
    }
    return attempt;
}

/* Places the card (within Move struct), assumes move is valid */
Board* place_card(Board* board, Move* move, int pNo, bool isBot) {
    // print for bots
    // printf("DEBUG: placing card\n");
    if(isBot) {
        // printf("DEBUG: is autobot\n");
        printf(A_PLAY, pNo, move->card, move->move[0], move->move[1]);
    }
    // new card
    Card* play = (Card*)malloc(sizeof(Card));
    play->value = atoi(&move->card[0]);
    play->suit = move->card[1];
    // place
    board->slots[move->move[0] - 1][move->move[1] - 1] = play;
    return board;
}

/* Returns true if any space on the board is free */
bool has_space(Board* board) {
    bool hasSpace = false;
    int i = 0;
    while(!hasSpace && i < board->dim[0]) {
        int j = 0;
        while(!hasSpace && j < board->dim[1]) {
            if(board->slots[i][j] == NULL) {
                hasSpace = true;
            }
            j++;
        }
        i++;
    }
    return hasSpace;
}

/* Returns true iff the slot selected is a valid place for a card (and is
 * free)
 */
bool is_valid(Board* board, Move* attempt, bool first) {
    if(attempt == NULL) {
        // printf("DEBUG: terminated input\n");
        return false;
    } else if(attempt->move == NULL) {
        // printf("DEBUG: invalid parameters\n");
        return false;
    } else if(attempt->move[0] < 1 || board->dim[0] < attempt->move[0]) {
        return false;
    } else if(attempt->move[1] < 1 || board->dim[1] < attempt->move[1]) {
        return false;
    } else if(!first) {
        // check adjacent tiles
        static int directions = 4;
        static int deltas[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
        bool flag = false;
        for(int i = 0; i < directions; i++) {
            if(slot_is_full(board, attempt->move, deltas[i])) {
                // an adjacent exist
                flag = true;
            }
        }
        if(!flag) {
            // no adjacent, invalid
            return false;
        }
    }
    // return true if tile is empty
    return board->slots[attempt->move[0] - 1][attempt->move[1] - 1] == NULL;
}

/* Checks if a slot on the board is empty (all indexes are +1 offset) */
bool slot_is_full(Board* board, int* source, int* delta) {
    int target[2];
    // check dx > 0 && < max(x)
    target[0] = (source[0] - 1) + delta[0];
    if(target[0] < 0) {
        target[0] = board->dim[0] - 1;
    } else if(target[0] >= board->dim[0]) {
        target[0] = 0;
    }
    // same for dy
    target[1] = (source[1] - 1) + delta[1];
    if(target[1] < 0) {
        target[1] = board->dim[1] - 1;
    } else if(target[1] >= board->dim[1]) {
        target[1] = 0;
    }
    // see if there's an adjacent card
    if(board->slots[target[0]][target[1]] == NULL) {
        return false;
    } else {
        return true;
    }
}
