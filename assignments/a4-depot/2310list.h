#ifndef __2310LIST_H__
#define __2310LIST_H__

/* Standard Libraries */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "2310util.h"

/* Public Definitions */

// Linked List CDT
struct LList {
    struct LList* next;
    char* entry;
};

/* Linked List CDT (borrowed concepts from COMP3506) */

/* Creates a new Linked List */
struct LList* llist(char* entry);
/* Appends to an existing Linked List */
struct LList* llist_enqueue(struct LList* list, char* item);
/* Gets the next item in the queue */
struct LList* llist_dequeue(struct LList* list, char* target);
/* Gets an item from a specific index */
char* llist_get(struct LList* list, int index);
/* Returns True if items is in Linked List CDT */
bool llist_contains(struct LList* list, char* item);
/* Free's entire Linked List recursively */
void llist_clear(struct LList* list);
/* Deletes a specific item from the list, regardless of its location */
struct LList* llist_delete(struct LList* head, char* item);
/* Returns the length of the linked list */
int llist_sizeof(struct LList* head);
/* Exports entire list into NULL trerminated String array */
char** export_llist(struct LList* list);
/* Prints the entire linked list */
void print_llist(struct LList* list, char* seperator);

#endif
