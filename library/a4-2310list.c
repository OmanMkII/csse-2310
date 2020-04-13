/* Imported Files [Header by default] */

#include "2310list.h"

/* List Concrete Data Type, courtesy of COMP3506 & my memory; To be used for
 * imported data, such as that of files, and generally unsorted data that can/
 * should be iterated through.
 *
 * Takes a char* to initialise a new instance and returns the head of the new
 * linked list.
 */
struct LList* llist(char* entry) {
    if(entry == NULL || strlen(entry) == 0) {
        entry = EMPTY_STRING;
    }
    struct LList* newList = (struct LList*)malloc(sizeof(struct LList));
    newList->next = NULL;
    newList->entry = (char*)malloc(sizeof(char) * STRING_BUFFER);
    memcpy(newList->entry, entry, sizeof(char) * STRING_BUFFER);
    return newList;
}

// Local Prototypes

struct LList* list_del_first(struct LList* head);
struct LList* delete_next(struct LList* prev);

/* Appends a new char* to an existing list, returns the head of the new list */
struct LList* llist_enqueue(struct LList* list, char* item) {
    struct LList* it = list;
    while(it->next != NULL && strcmp(it->entry, EMPTY_STRING)) {
        it = it->next;
    }
    // place in empty || new node
    if(it->entry == NULL) {
        it->entry = malloc(sizeof(char) * STRING_BUFFER);
        memcpy(it->entry, item, sizeof(char) * STRING_BUFFER);
    } else if(strcmp(it->entry, EMPTY_STRING) == 0) {
        memcpy(it->entry, item, sizeof(char) * STRING_BUFFER);
    } else if(it->next == NULL) {
        // it->next = malloc(sizeof(struct LList));
        it->next = llist(item);
    }
    return list;
}

/* Copies the first item of the list into the given char* reference and returns
 * reference to the new head of the list.
 */
struct LList* llist_dequeue(struct LList* list, char* target) {
    // WARNING: Valgrind throws loads of errors when I try do this
    memcpy(target, list->entry, sizeof(char) * STRING_BUFFER);
    struct LList* head = list_del_first(list);
    return head;
}

/* Takes a pointer to the head of a linked list and the index wanted, returns
 * the char* at said index, or NULL if the index exceeds the bounds.
 */
char* llist_get(struct LList* list, int index) {
    int i = 0;
    struct LList* it = list;
    while(i < index && it->next != NULL) {
        i++;
        it = it->next;
    }
    // might not be there
    if(i != index) {
        return NULL;
    } else {
        return it->entry;
    }
}

/* Takes a pointer to the head of a list and an item to compare all entries to,
 * returns true iff the list contains said item.
 */
bool llist_contains(struct LList* list, char* item) {
    for(struct LList* it = list; it != NULL; it = it->next) {
        if(strcmp(item, it->entry) == 0) {
            return true;
        }
    }
    return false;
}

/* Takes the head of any linked list and frees all data within it */
void llist_clear(struct LList* list) {
    if(list->next == NULL) {
        free(list->next);
        free(list->entry);
        list = NULL;
    } else {
        llist_clear(list->next);
        // free this instance
        free(list->next);
    }
}

/* Deletes the FIRST item of a Linked List, requires the head of the list to
 * avoid any messy pointers.
 */
struct LList* list_del_first(struct LList* head) {
    struct LList* newStart = head->next;
    // Free data
    free(head->entry);
    free(head);
    return newStart;
}

/* Takes any entry of the linked list and deletes the item after it, updating
 * pointers as necessary and returning the node given.
 */
struct LList* delete_next(struct LList* prev) {
    struct LList* next;
    if(prev->next != NULL) {
        // if there's more, bridge
        next = prev->next->next;
        free(prev->next->entry);
    } else {
        // else terminate
        next = NULL;
    }
    free(prev->next);
    prev->next = next;
    return prev;
}

/* Takes a linked list and char*, if said string is within the list the list is
 * updated and the string removed, returns the head of the new list.
 */
struct LList* llist_delete(struct LList* list, char* item) {
    struct LList* result = list;
    if(strcmp(list->entry, item) == 0) {
        result = list_del_first(list);
    } else {
        struct LList* ptr = list;
        while(ptr->next != NULL) {
            if(strcmp(ptr->next->entry, item) == 0) {
                ptr = delete_next(ptr);
                break;
            }
            ptr = ptr->next;
        }
    }
    return result;
}

/* Takes the head of a linked list and returns the absolute length of it */
int llist_sizeof(struct LList* head) {
    int len = 0;
    struct LList* it = head;
    while(it != NULL) {
        len++;
        it = it->next;
    }
    return len;
}

// Debugging

/* Takes a linked list as entry and a given seperator, then prints all entries
 * with the seperator between them.
 */
void print_llist(struct LList* list, char* seperator) {
    struct LList* it = list;
    while(it->next != NULL) {
        fprintf(stderr, "%s%s", it->entry, seperator);
        it = it->next;
    }
    fprintf(stderr, "%s", it->entry);
    fflush(stdout);
}
