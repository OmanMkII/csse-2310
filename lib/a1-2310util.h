#ifndef __2310UTIL_H__
#define __2310UTIL_H__

/* Standard Libraries */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

/* Commentary:
 *
 * The following library is based on my memory of what I used for 2310 last
 * year, since I can't recycle the code I've generalised the functions to make
 * standard ways of handling char* (struct String's here), and Linked Lists
 * (struct LList) for importing FILE's.
 *
 * Ideally, they'll all be super useful for me during the semester (assuming
 * they work..); if not, then at least I'm writing printf instead of python's
 * print, so there's that.
 *
 * I've noticed that there's a shocking volume of memory consumption from the
 * pointer's pointer's pointers, but for now it works without needing any
 * compression/scalilng back.
 */

#define CSSE2310 "Hell."

/* Public Static Definitions */

// Globals
#define END_STRING '\0'
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

// Status return from raise(), default to >100 for errors
typedef enum {
    RESOLVED = 0,
    ERROR = 100,
    UNEXPECTED_END = 101,
    BAD_REALLOC = 102,
    TTL_EXCEEDED = 103
} Status;

// Pseudo-errors
#define MSG_100 "Internal error\n"
#define MSG_101 "Unexpected end of array\n"
#define MSG_102 "Realloc failed\n"
#define MSG_103 "Message TTL reached zero with no resolution\n"

/* Public Definitions */

// Static integer array def'n for ease of use
typedef struct {
    int* array;
    int len;
} IntArray;

// A static definition of a String in the neolithic C99 language
// Arrays of String types should be NULL terminated
typedef struct {
    char* string;
    int len;
} String;

// Linked List CDT
struct LList {
    struct LList* next;
    char* entry;
};

// Dictionary data struct
struct Dict {
    // Stores entire list, as opposed to hash table (too lazy)
    struct nlist* data;
};

struct NList {
    // C Programming Language, 2nd Edition
    // Kernighan & Ritchie,
    // Also, Stack Overflow for referring me
    struct NList* next;
    char* key;
    //char* entry;
    void* entry;
};

// TODO: if pulling unknown response, alter a union type with an enum return

struct PipeClient {
    int* input;
    int* output;
};

struct Client {
    int portno;
    // TODO: C99 socket
};

// struct Socket {
//     // TODO: locking mechanisms
// };
//
// struct Data {
//     // TODO: locking mechanisms
// };

/* Public Functions */

/* Global Data Tpes:
 *
 * These following arrays are to be used in basically any and all applications
 * of Strings or int arrays to save me some SegFault headaches.
 *
 * Function Style: for all, len == 6 where proper
 *    -> for larger types (e.g. str arr, group to [type]_[function])
 *
 * [type]           | init new struct
 * [type][function] | general functions
 * [type]clr        | free entire object
 * exp[type]        | export to "classic" type [String -> char*]
 */

// Int array types

/* Creates a new array of integers as a solid struct type IntArray */
IntArray* intarr(int len, int input[]);
/* Extends the int array by the given number. */
IntArray* intarr_app(IntArray* target, int new);
/* Free's the full IntArray. */
void intarr_clr(IntArray* target);

// String struct types

/* Creates a new String type, i.e. a char array that's not so primitive */
String* string(char* input);
/* Appends a single char */
void strapp(String* target, char ch);
/* Free's the String type */
void strclr(String* target);
/* Exports the String data type to the classic (*cough* annoying) char* seen in
 * most C99 scripts.
 */
char* expstr(String* origin);

// Arrays of Strings coined "strarr" (String** that's NULL terminated)

/* Creates a new array of strings; no strings yet exist in this array if first
 * is NULL.
 */
String** strarr(int size, char** data);
/* Increments a string array by given size */
String** strarr_inc(String** target, int newsize);
/* Appends a new string array to the existing array */
String** strarr_ext(String** target, char** data, int len);
/* Appends a single String to the current array */
String** strarr_app(String** target, char* data);
/* Free's all elements of the String array */
void strarr_clr(String** target);
/* Returns the sizeof a String array; assumes all are NULL terminated */
int strarr_sizeof(String** array);
/* Exports the String Array to the classic char** array in most C99 scripts,
 * does not use NULL terminator (TODO: see if I need to).
 */
char** exp_strarr(String** origin);

// Linked List CDT (borrowed concepts from COMP3506)

/* Creates a new Linked List */
struct LList* llist(char* entry);
/* Appends to an existing Linked List */
struct LList* lstapp(struct LList* list, char* item);
/* Gets an item from a specific index */
char* lstget(struct LList* list, int index);
/* Returns True if items is in Linked List CDT */
bool lstcon(struct LList* list, char* item);
/* Free's entire Linked List recursively */
void lstclr(struct LList* list);
/* Deletes a specific item from the list, regardless of its location */
struct LList* lstdel(struct LList* head, char* item);
/* Exports entire list into NULL trerminated String array */
String** explst(struct LList* list);
/* Prints the entire linked list */
void printlst(struct LList* list);

/* General Functions */

/* Used to "raise" an error (set errno) and print a message to stderr in the
 * event that something goes wrong.
 */
void raise(char* message, int flag);

// Data Helper functions

/* Reads the next line of the file for processing; returns index of next char
 * for reference of next call (-1 denotes EOF).
 */
int readln(char* dest, char* file, int index);
/* Reads an entire file, and inputs it to a LList struct for iteration; i.e.
 * use a for loop to process the file after importing it here.
 */
struct LList* readfl(char* filename);

/* Splits a given string into an array based on the marker provided */
char** spltstr(char* input, char* marker);

/* Free char** array with NULL termination */
void free_array(char** target);

/* Returns true iff all characters of the argument are digits; used for when
 * input args are string forms.
 */
bool isnum(char* string);

/* Gets the index of the given char from the alphabet */
int get_index(char c);
/* Gets the indexed char from the alphabet */
char get_char(int i);

#endif
