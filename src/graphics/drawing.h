#ifndef DRAWING_H
#define DRAWING_H
#include <ncurses.h>
#include "entities.h"
#include "map.h"

#define JOLLY_PAIR 127

/* COLOR CODES */

enum color_codes {
    COLORCODES_CROC_B = 10,
    COLORCODES_CROC_A,
    COLORCODES_FROG_B,
    COLORCODES_FROG_A,
    COLORCODES_FLOWER_B,
    COLORCODES_FLOWER_A,
    COLORCODES_PROJECTILE_F,
    COLORCODES_PROJECTILE_FL,
    COLORCODES_WATER,
    COLORCODES_GRASS,
    COLORCODES_HIDEOUT,
    COLORCODES_SIDEWALK
};

typedef struct {
    int x;
    unsigned int y;
} Position;

typedef struct {
    enum entity_type type;
    Position pos;       // UP-LEFT CORNER
} Drawing;

typedef struct {
    unsigned int x;
    unsigned int y;
} Screen;

typedef struct {
    char *top;
    char *middle;
    char *bottom;
} EntityObject;

#endif





