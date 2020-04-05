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
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <semaphore.h>

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
 * After ass3, my LList was pretty useful to cycle through, so Ive standardised
 * it in a second library.
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
#define COLON ":"

// Constants used
#define EMPTY_STRING "\0"
#define STRING_SIZE 8
#define STRING_BUFFER 80
#define END_FILE -1

// General Statics
#define ALPHABET "ABCDEFGHIJKLMONPQRSTUVWXYZ"
#define ALPHABET_SIZE 26
#define NUM_CLIENTS 10
#define UNDEF_PORTNO "0"

/* Public Definitions */

// Network struct for a client
typedef struct {
    pthread_t tId;
} Thread;

// TCP Server, listens for new clients
typedef struct {
    sem_t lock;
    char* portno;
    int fd;
} Server;

// TCP Client, writes to a destination port
typedef struct {
    // main info
    int portno;
    // handshake bool
    bool wasPinged;
    // 1: read
    int fd1;
    FILE* read;
    // 2: write
    int fd2;
    FILE* write;
} Client;

/* General Functions */

/* Returns the sizeof an array that is NULL terminated */
int array_sizeof(char** array);

/* Returns true iff the input string contains the comparator from 0:SIZE */
bool starts_with(char* input, char* comparator);
/* Splits a given string into an array based on the marker provided */
char** split_string(char* input, char* marker);
/* Free char** array with NULL termination */
void free_array(char** target);

/* Returns true iff all characters of the argument are digits */
bool is_num(char* string);
/* Returns true iff all chars are hexdec characters as above */
bool is_xnum(char* string);

/* Gets the index of the given char from the alphabet */
int get_index(char c);
/* Gets the indexed char from the alphabet */
char get_char(int i);

// Threading
Thread* new_thread(void* (*routine)(void*), void* args);
void** close_thread(Thread* thread);
// Server sockets (listening)
Server* new_server(Server* sock, char* target);
Client* listen_socket(Server* sock);
void close_socket(Server* sock);
// Client sockets (connections)
Client* new_client(char* target, char* portno);
void write_socket(Client* sock, char* message);
char* read_socket(Client* sock);
void close_connection(Client* sock);

#endif
