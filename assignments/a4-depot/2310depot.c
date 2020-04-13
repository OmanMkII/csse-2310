/* 2310 Depot:
 *
 * A graph-based (well, the client of) network containing duplicates of this
 * client that communicate transfers, primarily based on IPv4 TCP communication
 * locally.
 */

/* Imported Files */

// C libraries
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

// Local libraries
#include "2310util.h"
#include "2310list.h"
#include "2310items.h"

/* Static Declarations */

// Common
#define LOCAL "127.0.0.1"
#define HANDSHAKE "IM:%s:%s\n"

// Input keys
#define NEGATIVE "-%d"

// Connect:p
#define CONNECTION "Connect"
#define CONNECTION_ARGS 2
// IM:p:name
#define INTRODUCTION "IM"
#define INTRODUCTION_ARGS 3
// Deliver:q:t
#define DELIVER "Deliver"
#define DELIVER_OUT "Deliver:%s:%s\n"
#define DELIVER_ARGS 3
// Withdraw:q:t
#define WITHDRAW "Withdraw"
#define WITHDRAW_ARGS 3
// Transfer:q:t:dest
#define TRANSFER "Transfer"
#define TRANSFER_ARGS 4
// Defer:k:Deliver:q:t
// Defer:k:Withdraw:q:t
// Defer:k:Transfer:q:t:dest
#define DEFER "Defer"
#define DEFER_ARGS 2
// Execute:k
#define EXECUTE "Execute"
#define EXECUTE_ARGS 2

/* Data Types */

// Primary data of the depot, holds basically everything
typedef struct {
    // central depot
    char* name;
    // depot data
    Data* data;
    // comms hub
    // Hub* hub;
    Server* sock;
    // semaphores for thread access
    // "What for?" you might ask, well here's why:
    // I've got the main data being locked and then unlocked as it gets altered
    // on a thread-by-thread case.
    // But, I don't want to write a queue CDT to help me here (statics were a
    // pain..), so instead I have a set of semaphores alternating if main() is
    // able to tamper with this buffer or not, and it sets them appropriately.
    sem_t canRead;
    sem_t canWrite;
    // the actual message
    char* message;
    // list of waiting commands
    // despite not wanting to write a queue, this is only used by main thread
    // so it actually holds quite well, YMMV
    struct LList* deferred;
} Depot;

// Tertiary data holder to save me some pain
struct ClientArg {
    Depot* depot;
    Client* sock;
};

/* Local Prototypes */

// Handler setup
void setup_handlers(Data* hubData);
// Handler thread runner
void* handle_signals(void* input);

// Raw input
void handle_input(Depot* depot, char* input);
// Input handlers
void new_connection(Depot* depot, char** connection);
void connection_reply(Depot* depot, char** im);
void recv_delivery(Depot* depot, char** delivery);
void make_withdrawal(Depot* depot, char** withdrawal);
void make_transfer(Depot* depot, char** transfer);
void defer_order(Depot* depot, char* deferred);
void finish_deferred(Depot* depot, char** key);

// Handlers
void setup_network(Depot* depot);
void* manage_network(void* data);
Server* spawn_server(Depot* depot);
Client* spawn_new_client(Depot* depot, char* portno);
void* spawn_client_thread(void* data);

/* Main Function */

int main(int argc, char** argv) {
    // We're no strangers to love
    if(argc < ARGS_MIN || argc % ARGS_FACTOR != 0) {
        // You know the rules and so do I
        fprintf(stderr, ERR_ARGS);
        exit(ERR_ARGS_CODE);
    } else if(!is_valid_name(argv[1])) {
        // A full commitment's what I'm thinking of
        fprintf(stderr, ERR_NAMES);
        exit(ERR_NAMES_CODE);
    } else {
        // You wouldn't get this from any other guy
        static Depot* depot;
        depot = malloc(sizeof(Depot));
        depot->name = malloc(sizeof(char) * STRING_BUFFER);
        sprintf(depot->name, "%s", argv[1]);
        // initialise items
        depot->data = initialise_data(argc, argv);
        // I just wanna tell you how I'm feeling
        setup_handlers(depot->data);
        // Gotta make you understand
        setup_network(depot);
        // no deferred commands
        depot->deferred = NULL;
        // set up semaphores (cannot yet read, but can write)
        // *always* waits before reading (see spawn_client_thread for notes)
        sem_init(&depot->canRead, 0, 0);
        sem_init(&depot->canWrite, 0, 1);
        // main thread
        char buffer[STRING_BUFFER] = EMPTY_STRING;
        depot->message = malloc(sizeof(char) * STRING_BUFFER);
        while(1) {
            // Never gonna give you up
            sem_wait(&depot->canRead);
            // Never gonna let you down
            memcpy(&buffer, depot->message, sizeof(char) * STRING_BUFFER);
            // Never gonna run around and desert you
            memset(depot->message, EMPTY_STRING[0],
                    sizeof(char) * STRING_BUFFER);
            // Never gonna make you cry
            sem_post(&depot->canWrite);
            // Never gonna say goodbye
            handle_input(depot, buffer);
            // Never gonna tell a lie and hurt you
        }
    }
    // one more flush, to be safe
    fflush(stdout);
    return 0;
}

/* Signal Handling */

// Takes a pointer to the main Data struct; launches a new thread to handle
// signals and passes the pointer along for use
void setup_handlers(Data* hubData) {
    // Signal set
    sigset_t masks;
    sigemptyset(&masks);
    sigaddset(&masks, SIGHUP);
    // we're also ignoring SIGPIPE for now
    sigaddset(&masks, SIGPIPE);
    // Block signals from main threads
    int s = pthread_sigmask(SIG_BLOCK, &masks, NULL);
    if(s != 0) {
        fprintf(stderr, "DEBUG: bad sigmask at signal handler setup\n");
    }
    // Signal handler thread
    pthread_t tid;
    s = pthread_create(&tid, NULL, &handle_signals, (void*)hubData);
    if(s != 0) {
        fprintf(stderr, "DEBUG: bad thread init at handler setup\n");
    }
}

// Primary signal handler takes a (void*)Data struct to print stock and
// neighbour data when SIGHUP has been received, used as a thread and returns
// zero iff terminated
void* handle_signals(void* input) {
    // handle SIGHUP && SIGPIPE
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGPIPE);
    // deal with main thread's variables
    Data* hubData = (Data*)input;
    int num;
    // printf("TODO: suppress all errors\n");
    while(!sigwait(&set, &num)) {
        if(num == SIGHUP) {
            print_stock(hubData);
            print_neighbours(hubData);
        } else if(num == SIGPIPE) {
            // printf("DEBUG: broken pipe\n");
        } else {
            // printf("DEBUG: got unknown signal (id %d)\n", num);
        }
    }
    return 0;
}

/* Local Functions */

// Handles all input from the main socket translated to char*, takes a pointer
// to the main depot to allow manipulation of data
void handle_input(Depot* depot, char* input) {
    char buffer[STRING_BUFFER] = EMPTY_STRING;
    memcpy(&buffer, input, sizeof(char) * STRING_BUFFER);
    char** order = split_string(buffer, COLON);
    int len = array_sizeof(order);
    // check what order it is
    if(strcmp(CONNECTION, order[0]) == 0 && len == CONNECTION_ARGS) {
        new_connection(depot, order);
    } else if(strcmp(INTRODUCTION, order[0]) == 0 &&
            len == INTRODUCTION_ARGS) {
        connection_reply(depot, order);
    } else if(strcmp(DELIVER, order[0]) == 0 && len == DELIVER_ARGS) {
        recv_delivery(depot, order);
    } else if(strcmp(WITHDRAW, order[0]) == 0 && len == WITHDRAW_ARGS) {
        make_withdrawal(depot, order);
    } else if(strcmp(TRANSFER, order[0]) == 0 && len == TRANSFER_ARGS) {
        make_transfer(depot, order);
    } else if(strcmp(DEFER, order[0]) == 0 && len > DEFER_ARGS) {
        // handle order checking internally
        defer_order(depot, input);
    } else if(strcmp(EXECUTE, order[0]) == 0 && len == EXECUTE_ARGS) {
        finish_deferred(depot, order);
    } else {
        // silently ignore
    }
}

// When 'Connect' is received, takes a reference to the depot and the arguments
// of the connect call to create a new socket to the target and begin handshake
void new_connection(Depot* depot, char** connection) {
    if(!is_num(connection[1])) {
        fprintf(stderr, "DEBUG: bad portno (got %s)\n", connection[1]);
    } else {
        Neighbour* ref = NULL;
        int i;
        for(i = 0; i < depot->data->nNeighbours; i++) {
            if(depot->data->neighbours[i].portno == atoi(connection[1])) {
                return;
            }
        }
        // whole new bit
        if(ref == NULL) {
            fflush(stdout);
            if(depot->data->neighbours == NULL) {
                depot->data->neighbours = malloc(sizeof(Neighbour));
            } else {
                depot->data->neighbours = realloc(depot->data->neighbours,
                        sizeof(Neighbour) * (depot->data->nNeighbours + 1));
            }
            ref = &depot->data->neighbours[depot->data->nNeighbours];
            ref->sock = NULL;
            ref->portno = atoi(connection[1]);
            depot->data->nNeighbours++;
        }
        ref->name = "<none>";
        // new sock to peer
        if(ref->sock == NULL) {
            Client* sock = spawn_new_client(depot, connection[1]);
            if(sock != NULL && !sock->wasPinged) {
                char reply[STRING_BUFFER] = EMPTY_STRING;
                sprintf(reply, HANDSHAKE, depot->sock->portno, depot->name);
                write_socket(sock, reply);
                sock->wasPinged = true;
                ref->sock = sock;
            }
        } else if(!ref->sock->wasPinged) {
            char reply[STRING_BUFFER] = EMPTY_STRING;
            sprintf(reply, HANDSHAKE, depot->sock->portno, depot->name);
            write_socket(ref->sock, reply);
            ref->sock->wasPinged = true;
        }
    }
}

// When a handshake ('IM') is received, takes a pointer to the depot and the
// message in a split array form to return the message in kind
void connection_reply(Depot* depot, char** im) {
    fflush(stdout);
    Neighbour* ref = NULL;
    int i;
    for(i = 0; i < depot->data->nNeighbours; i++) {
        if(depot->data->neighbours[i].portno == atoi(im[1])) {
            fflush(stdout);
            ref = &depot->data->neighbours[i];
            break;
        }
    }
    // whole new bit
    if(ref == NULL) {
        fflush(stdout);
        if(depot->data->neighbours == NULL) {
            depot->data->neighbours = malloc(sizeof(Neighbour));
        } else {
            depot->data->neighbours = realloc(depot->data->neighbours,
                    sizeof(Neighbour) * (depot->data->nNeighbours + 1));
        }
        ref = &depot->data->neighbours[depot->data->nNeighbours];
        ref->sock = NULL;
        ref->portno = atoi(im[1]);
        depot->data->nNeighbours++;
    }
    ref->name = im[2];
    // new sock to peer (can get a bad port, so don't ping if so)
    if(ref->sock == NULL) {
        Client* sock = spawn_new_client(depot, im[1]);
        if(sock != NULL && !sock->wasPinged) {
            char reply[STRING_BUFFER] = EMPTY_STRING;
            sprintf(reply, HANDSHAKE, depot->sock->portno, depot->name);
            write_socket(sock, reply);
            sock->wasPinged = true;
            ref->sock = sock;
        }
    } else if(!ref->sock->wasPinged) {
        char reply[STRING_BUFFER] = EMPTY_STRING;
        sprintf(reply, HANDSHAKE, depot->sock->portno, depot->name);
        write_socket(ref->sock, reply);
        ref->sock->wasPinged = true;
    }
}

// When a 'Delivery' message is received, takes a reference to the depot and a
// split version of the original message to adjust the appropriate stock
void recv_delivery(Depot* depot, char** delivery) {
    pthread_mutex_lock(&depot->data->lock);
    if(depot->data->items == NULL) {
        // we now have items
        depot->data->items = malloc(sizeof(Item));
        depot->data->items[0] = new_stock(delivery[2], delivery[1]);
        depot->data->nItems = 1;
    } else {
        // find index of item, or reach end
        int i = 0;
        while(i < depot->data->nItems &&
                strcmp(depot->data->items[i].name, delivery[2])) {
            i++;
        }
        // act appropriately
        if(i == depot->data->nItems) {
            // new stock
            depot->data->items = realloc(depot->data->items,
                    sizeof(Item) * (depot->data->nItems + 1));
            depot->data->items[i] = new_stock(delivery[2], delivery[1]);
            depot->data->nItems += 1;
        } else {
            // adjust existing
            depot->data->items[i].quantity += atoi(delivery[1]);
        }
    }
    pthread_mutex_unlock(&depot->data->lock);
}

// When a 'Withdraw' message is received, takes a pointer to the depot and the
// message split into an array to deduct the appropriate items
void make_withdrawal(Depot* depot, char** withdrawal) {
    pthread_mutex_lock(&depot->data->lock);
    if(depot->data->items == NULL) {
        // set withdrawal no.
        int value = atoi(withdrawal[1]);
        char deduction[STRING_BUFFER] = EMPTY_STRING;
        sprintf(deduction, NEGATIVE, value);
        // we now have items
        depot->data->items = malloc(sizeof(Item));
        depot->data->items[0] = new_stock(withdrawal[2], deduction);
        depot->data->nItems = 1;
    } else {
        // find index of item, or reach end
        int i = 0;
        while(i < depot->data->nItems &&
                strcmp(depot->data->items[i].name, withdrawal[2])) {
            i++;
        }
        // act appropriately
        if(i == depot->data->nItems) {
            // negate value
            int value = atoi(withdrawal[1]);
            char deduction[STRING_BUFFER] = EMPTY_STRING;
            sprintf(deduction, NEGATIVE, value);
            // new stock
            depot->data->items = realloc(depot->data->items,
                    sizeof(Item) * (depot->data->nItems + 1));
            depot->data->items[i] = new_stock(withdrawal[2], deduction);
            depot->data->nItems += 1;
        } else {
            // adjust existing
            depot->data->items[i].quantity -= atoi(withdrawal[1]);
        }
    }
    pthread_mutex_unlock(&depot->data->lock);
}

// When a 'Transfer' message is received, takes a pointer to the main depot and
// the split message to deduct as with 'Withdraw' and to send a reply if the
// neighbour is known
void make_transfer(Depot* depot, char** transfer) {
    // remove from local stock
    make_withdrawal(depot, transfer);
    // find the right port
    Neighbour* ref = NULL;
    for(int i = 0; i < depot->data->nNeighbours; i++) {
        if(strcmp(depot->data->neighbours[i].name, transfer[3]) == 0) {
            ref = &depot->data->neighbours[i];
            break;
        }
    }
    // if it exists, transfer
    if(ref != NULL) {
        char buffer[STRING_BUFFER] = EMPTY_STRING;
        sprintf(buffer, DELIVER_OUT, transfer[1], transfer[2]);
        write_socket(ref->sock, buffer);
    } else {
        // ignore
        // printf("DEBUG: neighbour '%s' not found\n", transfer[3]);
    }
}

// When a 'Defer' order is received, takes a pointer to the main depot and the
// entire message to enqueue in the 'to do' list
void defer_order(Depot* depot, char* deferred) {
    if(depot->deferred == NULL) {
        depot->deferred = llist(deferred);
    } else {
        depot->deferred = llist_enqueue(depot->deferred, deferred);
    }
}

// When an 'Execute' order is received, takes a pointer to the main depot and
// the array version of the arguments; dequeues any orders with the correct
// key and executes them (any other keyed orders are re-queued)
void finish_deferred(Depot* depot, char** key) {
    if(depot->deferred != NULL) {
        struct LList* newQueue = NULL;
        struct LList* oldQueue = depot->deferred;
        while(oldQueue != NULL) {
            char* buffer = oldQueue->entry;
            oldQueue = oldQueue->next;
            char** order = split_string(buffer, COLON);
            // check it's the right key
            if(strcmp(order[1], key[1]) == 0) {
                // execute
                if(strcmp(order[2], DELIVER) == 0 &&
                        array_sizeof(order) == 5) { // 5
                    recv_delivery(depot, order + 2);
                } else if(strcmp(order[2], WITHDRAW) == 0 &&
                        array_sizeof(order) == 5) { // 5
                    make_withdrawal(depot, order + 2);
                } else if(strcmp(order[2], TRANSFER) == 0 &&
                        array_sizeof(order) == 6) { // 6
                    make_transfer(depot, order + 2);
                } else {
                    // printf("DEBUG: order '%s' not recognised\n", buffer);
                }
            } else {
                // pass & enqueue
                if(newQueue == NULL) {
                    newQueue = llist(buffer);
                } else {
                    newQueue = llist_enqueue(newQueue, buffer);
                }
            }
        }
        // put unused orders back
        depot->deferred = newQueue;
    }
}

// Takes a pointer to the main depot data struct and initialises a new thread
// to listen on the main network channel which is stored in the main struct
void setup_network(Depot* depot) {
    // set up hub struct
    static Server* sock;
    sock = malloc(sizeof(Server));
    depot->sock = sock;
    sem_init(&depot->sock->lock, 0, 1);
    // lock, and only unlock in new thread when setup complete
    sem_wait(&depot->sock->lock);
    pthread_t tid;
    int s = pthread_create(&tid, NULL, &manage_network, (void*)depot);
    if(s != 0) {
        fprintf(stderr, "DEBUG: bad thread init at handler setup\n");
    }
    sem_wait(&depot->sock->lock);
    printf("%s\n", depot->sock->portno);
    fflush(stdout);
    sem_post(&depot->sock->lock);
}

// Used as the main function for the thread assigned to listen on the main port
// takes a (void*)pointer to the main depot data struct
void* manage_network(void* data) {
    Depot* depot = (Depot*)data;
    // set up sockets
    depot->sock = new_server(depot->sock, LOCAL);
    if(depot->sock == NULL) {
        fprintf(stderr, "DEBUG: bad socket setup\n");
        return NULL;
    }
    sem_post(&depot->sock->lock);
    // listen forever and return clients
    while(true) {
        // We've known each other for so long
        struct ClientArg* args = malloc(sizeof(struct ClientArg));
        // Your heart's been aching but you're too shy to say it
        Client* newClient = listen_socket(depot->sock);
        // Inside we both know what's been going on
        args->sock = newClient;
        // We know the game and we're gonna play it
        args->depot = depot;
        // spawn new client
        pthread_t tid;
        int s = pthread_create(&tid, NULL, &spawn_client_thread,
                (void*)args);
        if(s != 0) {
            fprintf(stderr, "DEBUG: bad thread spawn\n");
        }
    }
    printf("DEBUG: early end of listen() loop\n");
    return 0;
}

// Takes a port number and launches a new client to listen on it, which is also
// passed a pointer to the main depot struct, this data is returned as a Client
// struct
Client* spawn_new_client(Depot* depot, char* portno) {
    // new client
    Client* sock = new_client(LOCAL, portno);
    if(sock == NULL) {
        // printf("DEBUG: no sock available\n");
        return NULL;
    }
    // data for client
    struct ClientArg* args = malloc(sizeof(struct ClientArg));
    args->sock = sock;
    args->depot = depot;
    // new thread for client
    pthread_t tid;
    int s = pthread_create(&tid, NULL, spawn_client_thread, (void*)args);
    if(s != 0) {
        fprintf(stderr, "DEBUG: bad thread spawn\n");
    }
    return args->sock;
}

// The main running script for any client threads, takes a (void*)pointer to
// the main depot data struct and waits for any new data written to the
// associated socket.
// These threads all work with the main function using a pair of cyclic
// semaphores: if any input arrives then a thread locks canWrite to prevent
// the mirror being overwritten (and unlocks canRead), the main function then
// reads the input and unlocks canWrite after doing so (locking canRead to
// prevent it using the same input twice).
void* spawn_client_thread(void* data) {
    struct ClientArg* args = (struct ClientArg*)data;
    char buffer[STRING_BUFFER];
    while(true) {
        // wait for new input
        memset(&buffer, EMPTY_STRING[0], sizeof(char) * STRING_BUFFER);
        fgets(buffer, STRING_BUFFER, args->sock->read);
        // wait for main() to finish [default is ready]
        if(strlen(buffer) != 0) {
            // And if you ask me how I'm feeling
            sem_wait(&args->depot->canWrite);
            // Don't tell me you're too blind to see
            memcpy(args->depot->message, buffer, sizeof(char) * STRING_BUFFER);
            sem_post(&args->depot->canRead);
            // read is now flagged, but write is not
        }
        // now wait for next message
    }
}
