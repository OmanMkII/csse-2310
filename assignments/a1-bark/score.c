#include "score.h"

int r_score(Board* board, struct Score* score, int* loc);

/* Calculates the scores of each player based on best chain of cards found,
 * calls r_score() below to generate all possible paths.
 */
int* calc_scores(Board* board) {
    // Get all possible scores
    int scoreGrid[board->dim[0]][board->dim[1]];
    for(int i = 0; i < board->dim[0]; i++) {
        for(int j = 0; j < board->dim[1]; j++) {
            struct Score* nthScore = malloc(sizeof(struct Score));
            nthScore->score = 0;
            nthScore->path = (int**)malloc(sizeof(int*) * 2);
            nthScore->path[1] = NULL;
            nthScore->list = (char**)malloc(sizeof(char*) * 2);
            nthScore->list[1] = NULL;
            int location[] = {i, j};
            scoreGrid[i][j] = r_score(board, nthScore, location);
        }
    }
    // Map best scores
    int scores[ALPHABET_SIZE] = {0};
    for(int a = 0; a < board->dim[0]; a++) {
        for(int b = 0; b < board->dim[1]; b++) {
            if(scoreGrid[a][b] > scores[get_index(board->slots[a][b]->suit)]) {
                scores[get_index(board->slots[a][b]->suit)] = scoreGrid[a][b];
            }
        }
    }
    // Sort by types
    int* maxScores = (int*)malloc(sizeof(int) * 2);
    int player1[13];
    for(int l = 0; l < ALPHABET_SIZE; l += 2) {
        player1[l / 2] = scores[l];
        if(player1[l / 2] > maxScores[0]) {
            maxScores[0] = player1[l / 2];
        }
    }
    int player2[13];
    for(int m = 1; m < ALPHABET_SIZE; m += 2) {
        player2[m / 2] = scores[m];
        if(player2[m / 2] > maxScores[1]) {
            maxScores[1] = player2[m / 2];
        }
    }
    return maxScores;
}

/* Called recursively to query all adjacent coordindates (excluding precursor),
 * and generate a linked list of plausible travel routes. If no more options
 * exist and final tile is not the same suit as origin, returns NULL, else
 * continues until termination clause.
 */
int r_score(Board* board, struct Score* score, int* loc) {
    // get source
    int i = 0;
    while(score->path[i] != NULL) {
        i++;
        // worst case: four cards of the same score send me infintely looping
        if(i > board->dim[0] * board->dim[1]) {
            // TODO: find a better solution
            return -1;
        }
    }
    // check this is valid
    if(atoi(score->list[i - 1]) <= board->slots[loc[0]][loc[1]]->value) {
        score->path = realloc(score->path, sizeof(int**) * (score->score + 1));
        score->path[i] = loc;
        score->path[i + 1] = NULL;
        // check if terminator
        if(score->list[0][1] == score->list[i][1]) {
            return score->score;
        }
    } else {
        // this location is invalid
        return 0;
    }
    // set next paths map
    int results[] = {0, 0, 0, 0};
    int paths[4][2] = {{loc[0] - 1, loc[1]}, {loc[0], loc[1] - 1}, {loc[0] +
            1, loc[1]}, {loc[0], loc[1] + 1}};
    // ensure wrap-around
    if(paths[0][0] < 0) {
        paths[0][0] += board->dim[0];
    }
    if(paths[0][1] < 0) {
        paths[0][1] += board->dim[1];
    }
    // map paths
    int best = 0;
    for(int j = 0; j < GRID_PATHS; j++) {
        if(paths[i][0] == score->path[i - 1][0] &&
                paths[i][1] == score->path[i - 1][1]) {
            // don't backtrack
            continue;
        }
        results[i] = r_score(board, score, paths[i]);
        if(results[i] > best) {
            best = results[i];
        }
    }
    return best;
}
