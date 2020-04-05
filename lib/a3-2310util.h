#ifndef __2310UTIL_H__
#define __2310UTIL_H__

/* Standard Libraries */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>

/* Commentary:
 *
 * The following library is based on my memory of what I used for 2310 last
 * year, since I can't recycle the code I've generalised the functions to make
 * standard ways of common data types like char*, FILE* etc.
 *
 * After ass1, I found that my String and IntArray data types weren't as useful
 * as I'd hoped, so they've been deprecated for now: calling them will raise an
 * internal error.
 *
 * Library has also been modified to better comply with style guides, thanks
 * mum..
 */

#define CSSE2310 "Hell."

/* Public Static Definitions */

// Globals
#define END_STRING '\0'
#define BACKSPACE '\b'
#define NEW_LINE "\n"
#define BLANK ""
#define SPACE " "
#define COMMA ","

// Constants used
#define EMPTY_STRING "$"
#define STRING_SIZE 8
#define STRING_BUFFER 80
#define END_FILE -1

// General Statics
#define ALPHABET "ABCDEFGHIJKLMONPQRSTUVWXYZ"
#define ALPHABET_SIZE 26

// Pipes
#define FD_SLOTS 2

// formatted
#define CARD_FORMAT "%c.%x"

// Status return from raise(), default to >100 for library errors
typedef enum {
    RESOLVED = 0,
    ERROR = 100,
    UNEXPECTED_END = 101,
    BAD_REALLOC = 102,
    TTL_EXCEEDED = 103,
    DEPRECATED = 104
} Status;

// Pseudo-errors
#define MSG_100 "Internal error\n"
#define MSG_101 "Unexpected end of array\n"
#define MSG_102 "Realloc failed\n"
#define MSG_103 "Message TTL reached zero with no resolution\n"
#define MSG_104 "Internally deprecated function\n"

/* Public Definitions */

// Linked List CDT
struct LList {
    struct LList* next;
    char* entry;
};

// Pipe struct holding id and input/output channels
typedef struct {
    int* fd1;
    int* fd2;
    FILE* write;
    FILE* read;
    pid_t pid;
} PipeClient;

/* Linked List CDT (borrowed concepts from COMP3506) */

/* Creates a new Linked List */
struct LList* llist(char* entry);
/* Appends to an existing Linked List */
struct LList* llist_app(struct LList* list, char* item);
/* Appends string in a given format */
struct LList* llist_app_card(struct LList* list, char suit, int rank);
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

/* General Functions */

// /* Used to "raise" an error (set errno) and print a message to stderr in the
//  * event that something goes wrong from my libraries.
//  */
// void raise(char* message, int flag);

// Data Helper functions

/* Returns the sizeof an array that is NULL terminated */
int array_sizeof(char** array);

/* Reads the next line of the file for processing; returns index of next char
 * for reference of next call (-1 denotes EOF).
 */
int read_line(char* dest, char* file, int index);
/* Reads an entire file, and inputs it to a LList struct for iteration; i.e.
 * use a for loop to process the file after importing it here.
 */
struct LList* read_file(char* filename);

/* Returns true iff the input string contains the comparator from 0:SIZE */
bool starts_with(char* input, char* comparator);

/* Splits a given string into an array based on the marker provided */
char** split_string(char* input, char* marker, bool strip);

/* Free char** array with NULL termination */
void free_array(char** target);

/* Returns true iff all characters of the argument are digits; used for when
 * input args are string forms.
 */
bool is_num(char* string);
/* Returns true iff all chars are hexdec characters as above */
bool is_xnum(char* string);

/* Gets the index of the given char from the alphabet */
int get_index(char c);
/* Gets the indexed char from the alphabet */
char get_char(int i);

// Pipes for A3

/* Fork to new C99 program */
PipeClient* pipe_prog(char** argv, bool suppress);
/* Write to client stdin */
void write_pipe(PipeClient* client, char* message);
/* Read from client stdout */
char* read_pipe(PipeClient* client);
/* Signals a pipe to exit and frees all data */
void kill_pipe(PipeClient* client);

#endif
