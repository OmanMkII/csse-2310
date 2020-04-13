/* Imported Libraries (Header Default) */

#include "2310items.h"

/* Functions */

// Checks if any invalid characters exist in the given name for an Item,
// Neighbour, or Depot, and returns false if any are found, or length is zero.
bool is_valid_name(char* name) {
    if(strlen(name) == 0) {
        return false;
    }
    char* invalid = INVALID_CHARS;
    for(int i = 0; i < (int)strlen(name); i++) {
        for(int j = 0; j < (int)strlen(invalid); j++) {
            if(name[i] == invalid[j]) {
                return false;
            }
        }
    }
    // else
    return true;
}

// Initialises all data associated with the new depot as static, takes the
// command line arguments (and no. of) to build new items, if any exist.
Data* initialise_data(int numArgs, char** itemData) {
    // initialise statics
    static Data* data;
    data = malloc(sizeof(Data));
    static Item* items;
    static Neighbour* neighbours;
    neighbours = NULL;
    // init items, if any
    if(numArgs > 2) {
        items = malloc(sizeof(Item) * (numArgs - 2) / 2);
        for(int i = ITEMS_INDEX; i < numArgs; i += ARGS_FACTOR) {
            if(!is_valid_name(itemData[i])) {
                fprintf(stderr, ERR_NAMES);
                fflush(stdout);
                exit(ERR_NAMES_CODE);
            } else if(!is_num(itemData[i + 1])) {
                fprintf(stderr, ERR_QUANTITY);
                fflush(stdout);
                exit(ERR_QUANTITY_CODE);
            } else {
                items[(i - 2) / 2] = new_stock(itemData[i], itemData[i + 1]);
            }
        }
    } else {
        items = NULL;
    }
    // assemble & return
    pthread_mutex_init(&data->lock, NULL);
    pthread_mutex_lock(&data->lock);
    data->items = items;
    data->nItems = (items == NULL ? 0 : (numArgs - 2) / 2);
    data->neighbours = neighbours;
    data->nNeighbours = 0;
    pthread_mutex_unlock(&data->lock);
    return data;
}

// Takes name and quantity of a new item and adds it to the data struct given,
// returns the updated struct
Data* add_stock(Data* data, char* name, char* quantity) {
    Item item = new_stock(name, quantity);
    pthread_mutex_lock(&data->lock);
    data->items = realloc(data->items, sizeof(Item) * (data->nItems + 1));
    data->items[data->nItems++] = item;
    pthread_mutex_unlock(&data->lock);
    return data;
}

// Takes name and quantity of a new item and returns a struct representation
Item new_stock(char* name, char* quantity) {
    Item item;
    item.name = name;
    item.quantity = atoi(quantity);
    return item;
}

// Takes a name of a new Neighbour and adds it to the given data struct, gives
// the updated struct back
Data* add_neighbour(Data* data, char* name) {
    Neighbour neighbour = new_neighbour(name);
    pthread_mutex_lock(&data->lock);
    data->neighbours = realloc(data->neighbours,
            sizeof(Neighbour) * (data->nNeighbours + 1));
    data->neighbours[data->nNeighbours++] = neighbour;
    pthread_mutex_unlock(&data->lock);
    return data;
}

// Initialises a new neighbour struct with the given name
Neighbour new_neighbour(char* name) {
    Neighbour neighbour;
    neighbour.name = name;
    neighbour.sock = NULL;
    return neighbour;
}

// Compares two (void*)items and returns lexicographic result from strcmp() of
// their names
int lexicographic_stock(const void* itemA, const void* itemB) {
    const char* nameA = (const char*)((Item*)itemA)->name;
    const char* nameB = (const char*)((Item*)itemB)->name;
    return strcmp(nameA, nameB);
}

// Compares two (void*)neigbours and returns lexicographic result from strcmp()
// or their names
int lexicographic_neighbour(const void* neighbourA, const void* neighbourB) {
    const char* nameA = (const char*)((Neighbour*)neighbourA)->name;
    const char* nameB = (const char*)((Neighbour*)neighbourB)->name;
    return strcmp(nameA, nameB);
}

// Takes the hub's data sections and prints all currently known items in
// lexicographic order with quantity
void print_stock(Data* hubData) {
    printf(STOCK_HEAD);
    if(hubData->items == NULL) {
        // print nothing
    } else {
        // print stock
        pthread_mutex_lock(&hubData->lock);
        Item* stock = malloc(sizeof(Item) * hubData->nItems);
        for(int i = 0; i < hubData->nItems; i++) {
            stock[i] = hubData->items[i];
        }
        qsort(stock, hubData->nItems, sizeof(Item), &lexicographic_stock);
        for(int i = 0; i < hubData->nItems; i++) {
            if(stock[i].quantity != 0) {
                printf(STOCK_OUT, stock[i].name, stock[i].quantity);
            }
            // else ignore
        }
        pthread_mutex_unlock(&hubData->lock);
    }
    fflush(stdout);
}

// Takes the hub's data sections and prints all currently known neighbours in
// lexicographic order
void print_neighbours(Data* hubData) {
    printf(NEIGHBOURS_HEAD);
    if(hubData->neighbours == NULL) {
        // print nothing
    } else {
        // print neighbours
        pthread_mutex_lock(&hubData->lock);
        Neighbour* neighbours =
                malloc(sizeof(Neighbour) * hubData->nNeighbours);
        for(int i = 0; i < hubData->nNeighbours; i++) {
            neighbours[i] = hubData->neighbours[i];
        }
        qsort(neighbours, hubData->nNeighbours, sizeof(Neighbour),
                &lexicographic_stock);
        for(int i = 0; i < hubData->nNeighbours; i++) {
            printf(NEIGHBOUR_OUT, neighbours[i].name);
        }
        pthread_mutex_unlock(&hubData->lock);
    }
    fflush(stdout);
}
