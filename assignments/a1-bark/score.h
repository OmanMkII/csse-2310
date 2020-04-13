#ifndef __SCORE_H__
#define __SCORE_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "exit.h"

/* Static Definitiions */

#define GRID_PATHS 4

/* Data Types */

struct Score {
    int** path;
    int score;
    char** list;
};

/* Public Prototypes */

int* calc_scores(Board* board);

#endif
