#ifndef MAP_H
#define MAP_H

#define HEIGHT 15
#define ACTUAL_HEIGHT 45

#define WIDTH 20
#define ACTUAL_WIDTH 60

#define STARTING_Y 0 //TEMP MACROS: REPLACE WITH CALCULATED Y
#define STARTING_X 0 //TEMP MACROS: REPLACE WITH CALCULATED X

#define WATER_COLOR 51, 51, 255
#define GRASS_COLOR 0, 102, 0
#define HIDEOUT_COLOR 51, 25, 0
#define SIDEWALK_COLOR 204, 102, 0

enum areas {
    AREAS_SIDEWALK = 21,
    AREAS_WATER = 18,
    AREAS_GRASS = 19,
    AREAS_HIDEOUT = 20
};

enum area_sizes {
    AREASIZES_HIDEOUT = 6,
    AREASIZES_GRASS = 12,
    AREASIZES_WATER = 36,
    AREASIZES_SIDEWALK = 42
};


//todo REPLACE STARTING_X with a reference to the map' starting X
//todo REPLACE ACTUAL_WIDTH with a reference to the playable portion of map
#define CSI(x) (x) < STARTING_X ? -(x) : 0
#define CEI(x, l) (x) + (l) > STARTING_X + ACTUAL_WIDTH ? (x) + (l) - STARTING_X + ACTUAL_WIDTH : (x) + (l) 

#endif