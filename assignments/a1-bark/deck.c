/* The Deck of cards that the player/s will modify when plaing the main game,
 * contains appropriate print cycles as well.
 */

// Header File

#include "deck.h"

/* Prototypes */

struct LList* deal_next(void);

/* Functions */

/* Reads the full deckfile and returns a Linked List CDT of string entries for
 * printing if valid, else NULL.
 */
struct DeckHolder* read_deck(char* filepath, int deckIndex) {
    // Read deck file
    struct LList* deckfile = readfl(filepath);
    if(deckfile == NULL) {
        return NULL;
    }
    // Check cards && size valid
    int i = 0, line = 0, size;
    struct LList* deckHead;
    // int size;
    for(struct LList* it = deckfile; it != NULL; it = it->next) {
        if(line == 0) {
            // set deck size
            // printf("DEBUG: entry %d v. index %d\n", atoi(it->entry), deckIndex);
            if(!isnum(it->entry) || atoi(it->entry) < 0 ||
                    atoi(it->entry) < deckIndex) {
                lstclr(deckfile);
                return NULL;
            }
            size = atoi(it->entry);
        } else {
            // check valid card
            if(strlen(it->entry) != 2 || !isdigit(it->entry[0]) ||
                    !isalpha(it->entry[1])) {
                // wrong length
                lstclr(deckfile);
                return NULL;
            }
            if(i <= deckIndex) {
                deckHead = it;
            }
            i++;
        }
        line++;
    }
    // check valid length
    if(size != i) {
        lstclr(deckfile);
        return NULL;
    }
    // Restructure for return
    struct DeckHolder* deck = malloc(sizeof(struct DeckHolder));
    deck->deckList = deckfile->next;
    deck->deckIndex = deckHead;
    deck->deckSize = atoi(deckfile->entry);
    // Free empties
    free(deckfile->entry);
    free(deckfile);
    return deck;
}

/* Creates a new card instance from a string */
Card* new_card(char* rawCard) {
    Card* card = (Card*)malloc(sizeof(Card));
    card->value = atoi(&rawCard[0]);
    card->suit = rawCard[1];
    return card;
}

/* Calculates the deck size.
 *
 * Know anyone from NZ? I hear they like boasting about the size of their deck.
 */
int get_deck_size(struct LList* deck) {
    int size = 0;
    struct LList* it = deck;
    while(it != NULL) {
        size++;
        it = it->next;
    }
    return size;
}
