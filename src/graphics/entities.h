#ifndef ENTITIES_G_H
#define ENTITIES_G_H

#include "../utils/imports.h"

#define FROG_WIDTH 3
#define FROG_HEIGHT 3

#define CROC_MIN 6  // 2 times
#define CROC_MAX 15 // 5 times

#define FLOWER_WIDTH 3
#define FLOWER_HEIGHT 3

#define PROJ_WIDTH 1
#define PROJ_HEIGHT 1

enum entity_type {
    CROC_TYPE,
    CROC_ANGRY_TYPE,
    FLOWER_TYPE,
    FLOWER_HARMED_TYPE,
    FROG_TYPE,
    FROG_HARMED_TYPE,
    PROJ_FROG_TYPE,
    PROJ_FLOWER_TYPE
};

#define CROC_B_COLOR 51, 102, 0
#define CROC_A_COLOR 102, 0, 0
#define FROG_B_COLOR 153, 255, 51
#define FROG_A_COLOR 255, 51, 51
#define FLOWER_B_COLOR 255, 255, 102
#define FLOWER_A_COLOR 255, 102, 102

#define PROJECTILE_F_COLOR 0, 51, 25 // FROG
#define PROJECTILE_FL_COLOR 255, 255, 204 // FLOWER

#endif