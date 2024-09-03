#ifndef ENTITIES_H
#define ENTITIES_H

#define FROG_WIDTH 3
#define FROG_HEIGHT 3

#define CROC_MIN 6  // 2 times
#define CROC_MAX 15 // 5 times

#define PLANT_WIDTH 3
#define PLANT_HEIGHT 3

#define PROJ_WIDTH 1
#define PROJ_HEIGHT 1

#define CROC_B_COLOR 51, 102, 0
#define CROC_A_COLOR 102, 0, 0
#define FROG_B_COLOR 153, 255, 51       // 90, 247, 22
#define FROG_A_COLOR 255, 51, 51
#define PLANT_B_COLOR 255, 255, 102
#define PLANT_A_COLOR 255, 102, 102

#define PROJECTILE_F_COLOR 0, 51, 25 // FROG
#define PROJECTILE_FL_COLOR 255, 255, 204 // PLANT

#define ART_OF_LENGTH_3 3
#define ART_OF_LENGTH_1 1

static char *_FROG_PLAY_ART[ART_OF_LENGTH_3] = {
        "\\^/",
        "( )",
        "/ \\"
};

static char *_CROC_X3_PLAY_ART[ART_OF_LENGTH_3] = {
        "█████████",
        "█████████",
        "█████████"
};

static char *_CROC_X2_PLAY_ART[ART_OF_LENGTH_3] = {
        "██████",
        "██████",
        "██████"
};

static char *_PLANT_PLAY_ART[ART_OF_LENGTH_3] = {
        "\\^/",
        "-*-",
        "/|\\"
};

static char *_PROJECTILE_PLAY_ART[ART_OF_LENGTH_1] =  { '|' };

#endif
