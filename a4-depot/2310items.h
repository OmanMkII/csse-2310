#ifndef __2310ITEMS_H__
#define __2310ITEMS_H__

/* Imported Libraries */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "2310util.h"

/* Static Definitions */

// Errors
#define ERR_ARGS "Usage: 2310depot name {goods qty}\n"
#define ERR_ARGS_CODE 1
#define ERR_NAMES "Invalid name(s)\n"
#define ERR_NAMES_CODE 2
#define ERR_QUANTITY "Invalid quantity\n"
#define ERR_QUANTITY_CODE 3

// Args
#define ARGS_MIN 2
#define ARGS_FACTOR 2
#define ITEMS_INDEX 2

// Output
#define STOCK_HEAD "Goods:\n"
#define STOCK_OUT "%s %d\n"
#define STOCK_OUT_A "%s"
#define STOCK_OUT_B " %d"
#define NEIGHBOURS_HEAD "Neighbours:\n"
#define NEIGHBOUR_OUT "%s\n"

// Data Indexing
#define NEIGHBOUR 0
#define ITEM 1

// Invalids
#define INVALID_CHARS " :\n\r"

/* Data Types */

// Neighbouring warehouses
typedef struct {
    char* name;
    int portno;
    Client* sock;
} Neighbour;

// Stock items
typedef struct {
    char* name;
    int quantity;
} Item;

// Secondary data centre of depot
typedef struct {
    // mutex lock to ensure thread safety
    pthread_mutex_t lock;
    // data
    Item* items;
    int nItems;
    Neighbour* neighbours;
    int nNeighbours;
} Data;

/* Shared Prototypes */

// Setup
bool is_valid_name(char* name);
Data* initialise_data(int numItems, char** itemData);

// Data Input
Data* add_stock(Data* data, char* name, char* quantity);
Item new_stock(char* name, char* quantity);
Data* add_neighbour(Data* data, char* name);
Neighbour new_neighbour(char* name);

// Sorting
int lexicographic_stock(const void* itemA, const void* itemB);
int lexicographic_neighbour(const void* neighbourA, const void* neighbourB);

// Output
void print_stock(Data* hubData);
void print_neighbours(Data* hubData);

#endif
