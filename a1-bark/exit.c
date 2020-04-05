/* Handles the exiting of the program, mostly by free'ing memory and handling
 * error outputs.
 */

/* Libraries */

#include "exit.h"

/* Prototypes */

void free_game(Bark* game);

/* Functions */

/* Collects exit conditions and outputs error messages */
void exit_prog(int code, Bark* game) {
    fflush(stdout);
    // Exiting by input code
    switch(code) {
        case ARGS_NUM:
            fprintf(stderr, "%s\n%s\n", ERR_1A, ERR_1B);
            free_game(game);
            exit(1);

        case ARGS_TYPE:
            fprintf(stderr, "%s\n", ERR_2);
            free_game(game);
            exit(2);

        case FILE_DECK:
            fprintf(stderr, "%s\n", ERR_3);
            free_game(game);
            exit(3);

        case FILE_SAVE:
            fprintf(stderr, "%s\n", ERR_4);
            free_game(game);
            exit(4);

        case SHORT_DECK:
            fprintf(stderr, "%s\n", ERR_5);
            free_game(game);
            exit(5);

        case BOARD_FULL:
            fprintf(stderr, "%s\n", ERR_6);
            free_game(game);
            exit(6);

        case EOINPUT:
            fprintf(stderr, "%s\n", ERR_7);
            free_game(game);
            exit(7);

        default:
            free_game(game);
            exit(0);
    }
}

/* Free's all data allocated before exit */
void free_game(Bark* game) {
    if(game == NULL) {
        return;
    }
    if(game->players != NULL) {
        if(game->players[0] != NULL) {
            lstclr(game->players[0]->llHand);
            free(game->players[0]);
        }
        if(game->players[1] != NULL) {
            lstclr(game->players[1]->llHand);
            free(game->players[1]);
        }
        free(game->players);
    }

    // if(game->dim != NULL) {
    //     intarr_clr(game->dim);
    // }
    // printf("TODO: free all variables\n");
    free(game);
}
