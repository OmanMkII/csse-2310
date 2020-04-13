/*
 * A utility library that is based on my 2018 projects from CSSE2310, yes, they
 * didn't work that well.
 *
 * Ideally, the below can be used to fast-track my assignments with general
 * stuff like reading files, managing arrays etc.
 *
 * TODO: see if I can recycle my code from 2018 here.
 */

/* Imported Files [Header by default] */

#include "2310util.h"

/* Function Primitives */

/* List Concrete Data Type, courtesy of COMP3506 & my memory; To be used for
 * imported data, such as that of files, and generally unsorted data that can/
 * should be iterated through.
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

/* Appends to an existing Linked List */
struct LList* llist_app(struct LList* list, char* item) {
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

/* Appends a card in text format for printing to stdout when needed */
struct LList* llist_app_card(struct LList* list, char suit, int rank) {
    char buffer[STRING_BUFFER] = EMPTY_STRING;
    sprintf(buffer, CARD_FORMAT, suit, rank);
    return llist_app(list, buffer);
}

/* Gets an item from a specific index */
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

/* Returns True if item is in Linked List CDT */
bool llist_contains(struct LList* list, char* item) {
    for(struct LList* it = list; it != NULL; it = it->next) {
        if(strcmp(item, it->entry) == 0) {
            return true;
        }
    }
    return false;
}

/* Free's entire Linked List recursively */
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

/* Deletes the FIRST item of a Linked List: assumes you're not an idiot and
 * gave me the start of the list.
 */
struct LList* list_del_first(struct LList* head) {
    struct LList* newStart = head->next;
    // Free data
    free(head->entry);
    free(head);
    return newStart;
}

/* Deletes the next item of the Linked List */
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

/* Deletes a specific item from the list, regardless of its location */
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

/* Finds the absolute length of the list and returns it */
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

/* Prints the entire linked list, toStandard flags for stdout (else stderr) */
void print_llist(struct LList* list, char* seperator) {
    struct LList* it = list;
    while(it->next != NULL) {
        fprintf(stderr, "%s%s", it->entry, seperator);
        it = it->next;
    }
    fprintf(stderr, "%s", it->entry);
    fflush(stdout);
}

/* General assist functions */

/* Returns the sizeof an array that is NULL terminated */
int array_sizeof(char** array) {
    int i = 0;
    while(array[i] != NULL) {
        i++;
    }
    return i;
}

// /* Raises an error code in errno and prints to stderr */
// void raise(char* message, int flag) {
//     fprintf(stderr, "%s", message);
//     fflush(stderr);
//     errno = flag;
// }

/* Reads the next line of the file for processing, returns index of next */
int read_line(char* dest, char* file, int index) {
    // printf("Warning: pointer errors in code reading file\n");
    FILE* input = fopen(file, "r");
    int c = 0; // char being read
    int i = 0; // index in file
    int j = 0; // index of char*
    char str[STRING_BUFFER] = "";
    while((c = fgetc(input)) != EOF) {
        i++;
        if(i < index) {
            // printf("%d", i);
            continue;
        } else if(c == *NEW_LINE) {
            break;
        } else {
            // printf("%c", c);
            str[j++] = (char)c;
        }
    }
    // printf("\n");
    memcpy(dest, str, sizeof(char) * STRING_BUFFER);
    fclose(input);
    if(c == EOF) {
        return END_FILE;
    } else {
        return (i + 1);
    }
}

/* Reads an entire file, and inputs it to a LList struct for iteration */
struct LList* read_file(char* filename) {
    // if(access(filename, R_OK | W_OK) == -1) {
    if(access(filename, R_OK) == -1) {
        return NULL;
    }
    // Build LList
    struct LList* output = malloc(sizeof(struct LList));
    char* line = (char*)malloc(sizeof(char) * STRING_BUFFER);
    // memset(line, END_STRING, sizeof(char) * STRING_BUFFER);
    // String* line;
    int l = read_line(line, filename, 0);
    output = llist(line);
    // Append all lines to LList
    int i = 0;
    while((l = read_line(line, filename, l)) != END_FILE) {
        // append then clear memory
        llist_app(output, line);
        memset(line, END_STRING, sizeof(char) * STRING_BUFFER);
        i++;
    }
    free(line);
    return output;
}

/* Splits a given string into an array based on the marker provided, will also
 * strip new line chars if requested.
 */
char** split_string(char* input, char* marker, bool strip) {
    // New NULL term. array
    int i = 0;
    char** output = (char**)malloc(sizeof(char*) * 2);
    // output[0] = (char*)malloc(sizeof(char) * STRING_BUFFER);
    char part[STRING_BUFFER] = "";
    // split it up
    char* token = strtok(input, marker);
    while(token != NULL) {
        // write to memory
        sprintf(part, "%s", token);
        output[i] = malloc(sizeof(char) * STRING_BUFFER);
        memcpy(output[i++], part, sizeof(char) * STRING_BUFFER);
        // NULL terminate array
        output = (char**)realloc(output, sizeof(char*) * (i + 1));
        output[i] = NULL;
        // get next part, nullify pointer
        part[0] = 0;
        token = strtok(NULL, marker);
    }
    // Strip trailing new line
    if(strip && output[i - 1][strlen(output[i - 1]) - 1] == *NEW_LINE) {
        output[i - 1][strlen(output[i - 1]) - 1] = END_STRING;
    }
    free(token);
    return output;
}

/* Compares the input with the comparator and returns true iff all characters
 * match.
 */
bool starts_with(char* input, char* comparator) {
    int i = 0;
    while(i != (int)strlen(comparator) && i != (int)strlen(input)) {
        if(input[i] != comparator[i]) {
            return false;
        }
        i++;
    }
    return true;
}

/* Free char** array with NULL termination */
void free_array(char** target) {
    int i = 0;
    while(target[i] != NULL) {
        free(target[i++]);
    }
    free(target);
}

/* Returns true iff all characters of the argument are digits */
bool is_num(char* string) {
    bool result = true;
    for(int i = 0; i < (int)strlen(string); i++) {
        if(!isdigit(string[i])) {
            result = false;
        }
    }
    return result;
}

/* Returns true iff all characters of the argument are hex digits */
bool is_xnum(char* string) {
    bool result = true;
    for(int i = 0; i < (int)strlen(string); i++) {
        if(!isxdigit(string[i])) {
            result = false;
        }
    }
    return result;
}

/* Gets the index of the given char from the alphabet */
int get_index(char c) {
    for(int i = 0; i < ALPHABET_SIZE; i++) {
        if(toupper(c) == ALPHABET[i]) {
            return c;
        }
    }
    return -1;
}

/* Gets the indexed char from the alphabet */
char get_char(int index) {
    return ALPHABET[index];
}

/* A3: Pipes & Forks */

/* Forks to a new C99 subprocess and returns a pipe struct that contains the
 * necessary data to communicate with it; takes a suppress bool for dev to
 * enable stderr output from client - should be suppressed for final.
 */
PipeClient* pipe_prog(char** argv, bool suppress) {
    // new data struct
    PipeClient* client = malloc(sizeof(PipeClient));
    client->fd1 = malloc(sizeof(int) * 2);
    client->fd2 = malloc(sizeof(int) * 2);
    // make pipes
    if(pipe(client->fd1) == -1 || pipe(client->fd2) == -1) {
        // failed pipe
        fprintf(stderr, "DEBUG: bad pipe\n");
        fflush(stderr);
    }
    // fork (assumes no fork errors)
    if((client->pid = fork()) == 0) {
        // pipe to child
        close(client->fd1[1]);
        close(client->fd2[0]);
        dup2(client->fd1[0], STDIN_FILENO);
        dup2(client->fd2[1], STDOUT_FILENO);
        // suppress stderr (pipe to the ether)
        if(suppress) {
            int err[2];
            pipe(err);
            close(err[1]);
            dup2(err[0], STDERR_FILENO);
        }
        //
        execv(argv[0], argv);
        // failsafe
        fprintf(stderr, "DEBUG: bad launch of %s with %d\n", argv[0], errno);
        fflush(stderr);

    } else {
        // parent
        close(client->fd1[0]);
        close(client->fd2[1]);
        client->write = fdopen(client->fd1[1], "w");
        client->read = fdopen(client->fd2[0], "r");
    }
    // TODO: ensure client exec success
    return client;
}

/* Write to the pipe via stdin */
void write_pipe(PipeClient* client, char* message) {
    fprintf(client->write, "%s", message);
    fflush(client->write);
}

/* Read output to stdout from the pipe client in the given string format */
char* read_pipe(PipeClient* client) {
    char* buffer = malloc(sizeof(char) * STRING_BUFFER);
    buffer = "\0";
    int state = fscanf(client->read, "%s", buffer);
    if(state == -1) {
        // free(buffer);
        return NULL;
    } else {
        return buffer;
    }
}

/* Signals to an existing pipe to terminate, and clears all data associated
 * with the client struct.
 */
void kill_pipe(PipeClient* client) {
    printf("TODO: close fd1, fd2\n");
    printf("TODO: close files too\n");
    fflush(stdout);
    // try wait(&pid) ?
    client == NULL ? printf("\n") : printf("\n");
    fflush(stdout);
}
