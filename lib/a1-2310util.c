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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "2310util.h"

/* Function Primitives */

char* string_copy(char* input);

// TODO: pipe and fork for A3, networking for A4
// TODO: consider using enum for their return statements

// Pipe to new sub program
void pipe_prog(char** argv, int* fd1, int* fd2);

// Network to new sub program (all using localhost)
// TODO: make a struct for networking
//void new_tcp(struct Client target, int portno);

/* IntArray Data Type: A struct housed array of ints */
IntArray* intarr(int len, int input[]) {
    IntArray* array = (IntArray*)malloc(sizeof(IntArray));
    array->len = len;
    array->array = (int*)malloc(sizeof(int) * len);
    if(input != NULL) {
        for(int i = 0; i < len; i++) {
            array->array[i] = input[i];
        }
    }
    return array;
}

/* Extends the int array by the given number. */
IntArray* intarr_app(IntArray* target, int new) {
    target->array = (int*)realloc(target->array,
            sizeof(int) * (target->len + 1));
    target->array[target->len++] = new;
    return target;
}

/* Free's the full IntArray. */
void intarr_clr(IntArray* target) {
    free(target->array);
    free(target);
}

/* String Data Type:
 *
 * A replacement for the C style primitive char arrays that I hate so much.
 */
String* string(char* input) {
    String* str = (String*)malloc(sizeof(String));
    // Unknown length
    str->len = 0;
    while(input[str->len] != '\0') {
        str->len++;
    }
    // For all chars
    str->string = (char*)malloc(sizeof(char) * (str->len + 1));
    memcpy(str->string, input, (str->len + 1));
    return str;
}

/* Appends a single char */
void strapp(String* target, char ch) {
    if(strcmp(target->string, EMPTY_STRING) == 0) {
        target->string[0] = ch;
    } else {
        target->len++;
        target->string = (char*)realloc(target->string,
                sizeof(char) * (target->len + 1));
        target->string[target->len - 1] = ch;
        target->string[target->len] = END_STRING;
    }
}

/* Free's the String type */
void strclr(String* target) {
    free(target->string);
    free(target);
}

/* Exports the String data type to the classic char* array */
char* expstr(String* origin) {
    char* out = (char*)malloc(sizeof(char) * origin->len);
    memcpy(out, origin->string, sizeof(char) * origin->len);
    return out;
}

/* String Array data type:
 *
 * To be used for all general purpose String arrays, such as visual displays
 * or objects.
 */
String** strarr(int size, char** data) {
    String** array = (String**)malloc(sizeof(String*) * (size + 1));
    for(int i = 0; i < size; i++) {
        // array[i] = (String*)malloc(sizeof(String));
        if(strcmp(data[i], BLANK) == 0) {
            array[i] = string(EMPTY_STRING);
        } else {
            char* entry = malloc(sizeof(char) * (strlen(data[i]) + 1));
            memcpy(entry, data[i], strlen(data[i]) + 1);
            array[i] = string(entry);
            free(entry);
        }
    }
    array[size] = NULL;
    return array;
}

/* Increments a string array by given size */
String** strarr_inc(String** target, int newsize) {
    int oldsize = strarr_sizeof(target);
    int size = oldsize + newsize;
    target = (String**)realloc(target, sizeof(String*) * (size + 1));
    // Append
    free(target[oldsize]);
    for(int i = oldsize; i < (size + 1); i++) {
        if(i != size) {
            target[i] = string(EMPTY_STRING);
        } else {
            target[i] = NULL;
        }
    }
    // target[size] = NULL;
    return target;
}

/* Appends a new string array to the existing array */
String** strarr_ext(String** target, char** data, int len) {
    // Fill all data in EMPTY_STRING || end of array (NULL)
    int l = 0;
    while(target[l] != NULL) {
        l++;
    }
    target = realloc(target, sizeof(String*) * ((l + 1) + len));
    int i = 0, j = 0;
    while(j < len) {
        if(target[i] == NULL) {
            target = strarr_inc(target, 1);
            free(target[i]);
            target[i] = string(data[j++]);
        } else if(strcmp(target[i]->string, EMPTY_STRING) == 0) {
            strclr(target[i]);
            target[i] = string(data[j++]);
        } else {
            // skip full item
        }
        i++;
    }
    // target[l + len] = NULL;
    return target;
}

/* Appends a single String to the current array */
String** strarr_app(String** target, char* data) {
    int i = 0;
    while(target[i] != NULL) {
        if(strcmp(target[i]->string, EMPTY_STRING) == 0) {
            // Found empty entry
            char* entry = malloc(sizeof(char) * (strlen(data) + 1));
            memset(entry, *data, strlen(data) + 1);
            target[i] = string(entry);
            return target;
        }
        i++;
    }
    // Otherwise, extend
    target = strarr_inc(target, 1);
    target[i] = string(data);
    return target;
}

/* Free's all elements of the String array */
void strarr_clr(String** target) {
    int i = 0;
    while(target[i] != NULL) {
        strclr(target[i]);
        // free(target[i]);
        i++;
    }
    free(target[i]);
    free(target);
}

/* Returns the sizeof a String array; assumes all are NULL terminated */
int strarr_sizeof(String** array) {
    int i = 0;
    while(array[i] != NULL) {
        i++;
    }
    return i;
}

/* Exports the String Array to the classic char** array */
char** exp_strarr(String** origin) {
    int len = strarr_sizeof(origin);
    char** output = malloc(sizeof(char*) * len);
    for(int i = 0; i < len; i++) {
        output[i] = malloc(sizeof(char*) * origin[i]->len);
        memcpy(output[i], origin[i]->string,
                sizeof(char) * (origin[i]->len + 1));
    }
    return output;
}

// Local Prototypes

struct LList* list_del_first(struct LList* head);
struct LList* delete_next(struct LList* prev);

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

/* Appends to an existing Linked List */
struct LList* lstapp(struct LList* list, char* item) {
    struct LList* it = list;
    while(it->next != NULL && strcmp(it->entry, EMPTY_STRING)) {
        it = it->next;
    }
    // place in empty || new node
    if(strcmp(it->entry, EMPTY_STRING) == 0) {
        memcpy(it->entry, item, sizeof(char) * STRING_BUFFER);
    } else if(it->next == NULL) {
        // it->next = malloc(sizeof(struct LList));
        it->next = llist(item);
    }
    return list;
}

/* Gets an item from a specific index */
char* lstget(struct LList* list, int index) {
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

/* Returns True if items is in Linked List CDT */
bool lstcon(struct LList* list, char* item) {
    for(struct LList* it = list; it->next != NULL; it = it->next) {
        if(strcmp(item, it->entry) == 0) {
            return true;
        }
    }
    return false;
}

/* Free's entire Linked List recursively */
void lstclr(struct LList* list) {
    if(list->next == NULL) {
        free(list->next);
        free(list->entry);
        list = NULL;
    } else {
        lstclr(list->next);
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
struct LList* lstdel(struct LList* list, char* item) {
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

/* Exports entire list into NULL trerminated String array */
// TODO: export to string lists
String** explst(struct LList* list);

// Debugging

/* Prints the entire linked list */
void printlst(struct LList* list) {
    struct LList* it = list;
    while(it != NULL) {
        printf("%s, ", it->entry);
        it = it->next;
    }
    printf(NEW_LINE);
}

/* General assist functions */

/* Raises an error code in errno and prints to stderr */
void raise(char* message, int flag) {
    fprintf(stderr, "%s", message);
    errno = flag;
}

/* Reads the next line of the file for processing, returns index of next */
int readln(char* dest, char* file, int index) {
    // printf("Warning: pointer errors in code reading file\n");
    FILE* f = fopen(file, "r");
    int c = 0; // char being read
    int i = 0; // index in file
    int j = 0; // index of char*
    char str[STRING_BUFFER] = "";
    while((c = fgetc(f)) != EOF) {
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
    fclose(f);
    if(c == EOF) {
        return END_FILE;
    } else {
        return (i + 1);
    }
}

/* Reads an entire file, and inputs it to a LList struct for iteration */
struct LList* readfl(char* filename) {
    // if(access(filename, R_OK | W_OK) == -1) {
    if(access(filename, R_OK) == -1) {
        return NULL;
    }
    // Build LList
    struct LList* output = malloc(sizeof(struct LList));
    char* line = (char*)malloc(sizeof(char) * STRING_BUFFER);
    // memset(line, END_STRING, sizeof(char) * STRING_BUFFER);
    // String* line;
    int l = readln(line, filename, 0);
    output = llist(line);
    // Append all lines to LList
    int i = 0;
    while((l = readln(line, filename, l)) != END_FILE) {
        // append then clear memory
        lstapp(output, line);
        memset(line, END_STRING, sizeof(char) * STRING_BUFFER);
        i++;
    }
    free(line);
    return output;
}

/* Splits a given string into an array based on the marker provided */
char** spltstr(char* input, char* marker) {
    // New NULL term. array
    int i = 0;
    char** output = (char**)malloc(sizeof(char*) * 2);
    output[0] = (char*)malloc(sizeof(char) * STRING_BUFFER);
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
    if(output[i - 1][strlen(output[i - 1]) - 1] == *NEW_LINE) {
        output[i - 1][strlen(output[i - 1]) - 1] = END_STRING;
    }
    return output;
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
bool isnum(char* string) {
    bool result = true;
    for(int i = 0; i < (int)strlen(string); i++) {
        if(!isdigit(string[i])) {
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
char get_char(int i) {
    return ALPHABET[i];
}

/* Returns the date of tomorrow; an easter egg for those who read this far.
 */
#include <time.h>

char* get_date(void);
void get_tomorrows_date(void);

/* Gets the date of today. */
char* get_date(void) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char* date = malloc(sizeof(char) * 16);
    sprintf(date, "%d/%d/%d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    return date;
}

/* Gets the date of tomorrow, today++ */
void get_tomorrows_date(void) {
    char* todaysDate = get_date();
    printf("Today: %s%s", todaysDate, NEW_LINE);
    sleep(1000 * 60 * 60 * 24);
    char* tomorrowsDate = get_date();
    printf("Tomorrow: %s%s", tomorrowsDate, NEW_LINE);
}
