/*
 * A utility library that is based on my 2018 projects from CSSE2310, yes, they
 * didn't work that well.
 *
 * For A4, I re-styled the Linked List (see 2310list) to take an unknown data
 * type, since going around in circles was so fun for A3.
 */

/* Imported Files [Header by default] */

#include "2310util.h"

/* General assist functions */

// Returns the length of a NULL terminated char** array for iteration
int array_sizeof(char** array) {
    int i = 0;
    while(array[i] != NULL) {
        i++;
    }
    return i;
}

// Splits a char* into a NULL terminated array of char* where each entry is a
// substring of the input seperated by the marker provided
char** split_string(char* input, char* marker) {
    // New NULL term. array
    int i = 0;
    char** output = (char**)malloc(sizeof(char*) * 2);
    char part[STRING_BUFFER] = EMPTY_STRING;
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
    free(token);
    return output;
}

// Compares the input string with the comparator string and returns true iff
// the input conatins all the chars of the comparator in order from 0
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

// Free a full char** up to a NULL terminator
void free_array(char** target) {
    int i = 0;
    while(target[i] != NULL) {
        free(target[i++]);
    }
    free(target);
}

// Returns true iff all characters of the string input are digits
bool is_num(char* string) {
    if(strlen(string) == 0) {
        return false;
    }
    for(int i = 0; i < (int)strlen(string); i++) {
        if(!isdigit(string[i])) {
            return false;
        }
    }
    return true;
}

// Returns true iff all characters of the argument are hex digits
bool is_xnum(char* string) {
    if(strlen(string) == 0) {
        return false;
    }
    for(int i = 0; i < (int)strlen(string); i++) {
        if(!isxdigit(string[i])) {
            return false;
        }
    }
    return true;
}

// Gets the index of the given char from the alphabet
int get_index(char c) {
    for(int i = 0; i < ALPHABET_SIZE; i++) {
        if(toupper(c) == ALPHABET[i]) {
            return c;
        }
    }
    return -1;
}

// Gets the ith char of the alphabet
char get_char(int index) {
    return ALPHABET[index];
}

/* A4: Networking */

// Builds a new thread for networking
Thread* new_thread(void* (*routine)(void*), void* args) {
    Thread* thread = malloc(sizeof(Thread));
    pthread_create(&thread->tId, NULL, routine, args);
    return thread;
}

// Closes and existing thread and returns pointer to the thread's return values
void** close_thread(Thread* thread) {
    void* returned;
    pthread_join(thread->tId, &returned);
    return returned;
}

// Takes a pointer to a malloc'd server memory block and initialises it to the
// given (char*)portno, returns the new Server struct
Server* new_server(Server* sock, char* target) {
    // static Server* sock;
    // sock = malloc(sizeof(Server));
    // find target
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int err;
    if((err = getaddrinfo(target, UNDEF_PORTNO, &hints, &ai)) != 0) {
        freeaddrinfo(ai);
        fprintf(stderr, "%s\n", gai_strerror(err));
        return NULL;
    }
    // build connection
    sock->fd = socket(AF_INET, SOCK_STREAM, 0);
    if(bind(sock->fd, ai->ai_addr, sizeof(struct sockaddr))) {
        fprintf(stderr, "DEBUG: bad socket binding\n");
        return NULL;
    }
    // get local address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    socklen_t len = sizeof(struct sockaddr_in);
    if(getsockname(sock->fd, (struct sockaddr*)&addr, &len)) {
        fprintf(stderr, "DEBUG: bad sock name\n");
        return NULL;
    }
    sock->portno = malloc(sizeof(char) * STRING_BUFFER);
    sprintf(sock->portno, "%u", ntohs(addr.sin_port));
    // listen on port
    // how will you learn if you do not listen?
    if((err = listen(sock->fd, NUM_CLIENTS)) != 0) {
        fprintf(stderr, "DEBUG: bad listen attempt\n");
        return NULL;
    }
    return sock;
}

// Listens for a short period on an existing Server and builds a new Client
// socket from the information received to return, intended for use within a
// loop functionality
Client* listen_socket(Server* sock) {
    int fd = accept(sock->fd, NULL, NULL);
    // new client
    Client* client;
    client = malloc(sizeof(Client));
    fflush(stdout);
    // init comms
    client->portno = -1;
    client->fd1 = fd;
    client->read = fdopen(client->fd1, "r");
    client->fd2 = dup(fd);
    client->write = fdopen(client->fd2, "w");
    return client;
}

// Closes a passive Server socket
void close_socket(Server* sock) {
    if(sock == NULL) {
        printf("DEBUG: null sock\n");
    }
    printf("TODO: close listening socket\n");
}

// Builds a new Client struct to the given address and port number
Client* new_client(char* target, char* portno) {
    Client* sock = malloc(sizeof(Client));
    sock->portno = atoi(portno);
    sock->wasPinged = false;
    // find address of target
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int err;
    if((err = getaddrinfo(target, portno, &hints, &ai)) != 0) {
        freeaddrinfo(ai);
        fprintf(stderr, "%s\n", gai_strerror(err));
        return NULL;
    }
    // build connection
    sock->fd1 = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(sock->fd1, ai->ai_addr, sizeof(struct sockaddr))) {
        // suppressing this for a4
        // fprintf(stderr, "DEBUG: bad socket connection\n");
        return NULL;
    }
    // open streams
    sock->read = fdopen(sock->fd1, "r");
    sock->fd2 = dup(sock->fd1);
    sock->write = fdopen(sock->fd2, "w");
    return sock;
}

// Writes a char* to an existing client and fflushes it to be sure
void write_socket(Client* sock, char* message) {
    fputs(message, sock->write);
    fflush(sock->write);
}

// Reads an existing Client socket and returns the char* data
char* read_socket(Client* sock) {
    char* buffer = malloc(sizeof(char) * STRING_BUFFER);
    fgets(buffer, STRING_BUFFER, sock->read);
    if(strlen(buffer) == 0) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

// Closes the given client socket and free's all data
void close_connection(Client* sock) {
    if(sock == NULL) {
        printf("DEBUG: null sock\n");
    }
    printf("TODO: close client\n");
}
