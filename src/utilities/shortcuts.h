#ifndef FROGGER_SHORTCUTS_H
#define FROGGER_SHORTCUTS_H

#include "../commons/structures.h"
#include "../commons/imports.h"


// Il valore di terminatore di stringa. ("\0")
#define TERM 1

// malloc() - Alloca la memoria.
#define MALLOC(type, size) (type *)malloc(sizeof(type) * (size))

// calloc() - Alloca e azzera la memoria.
#define CALLOC(type, size) (type *)calloc((size), sizeof(type))

// realloc() - Rialloca la memoria.
#define REALLOC(type, arr, size) (type *)realloc(arr, sizeof(type) * (size))

// memcpy() - Copia la memoria da una parte a un'altra.
#define MEMCPY(from, to, type, size) memcpy(to, from, sizeof(type) * (size))


// Alloca la memoria (malloc()) con terminatore.
#define MALLOC_TERM(type, size) (type *)malloc(sizeof(type) * (size + TERM))
// Alloca la memoria (calloc()) con terminatore.
#define CALLOC_TERM(type, size) (type *)calloc((size + TERM), sizeof(type))
// Rialloca la memoria (realloc()) con terminatore.
#define REALLOC_TERM(type, arr, size) (type *)realloc(arr, (sizeof(type) * size) + TERM)

/**
 * Crasha il programma se `pointer = NULL`.
 * Da utilizzare sempre dopo l'allocazione dinamica.
 */
#define CRASH_IF_NULL(pointer)                  \
    if (!pointer) {                             \
        perror("Cannot allocate memory!\n");    \
        exit(EXIT_FAILURE);                     \
    }

/**
 * Sleep function.
 */

typedef enum
{
    TIMEFRAME_MICROS,
    TIMEFRAME_MILLIS,
    TIMEFRAME_SECONDS
} TimeFrame;

void sleepy(int quantity, TimeFrame timeFrame);


/**
 * Number utils.
 */

int gen_num(int min, int max);
int choose_between(int count, ...);
bool choose_between_bool();

/**
 * String utils.
 */

char *concat(int n, ...);
bool str_eq(char *expected, char *toCompare);

enum AVAILABLE_ARTS {
    ART_BIG_FROG,
    ART_TWO_FROGS,
    ART_PAUSE_LOGO,
    ART_MAIN_LOGO,
    ART_UNKNOWN
};

int *get_screen_size();
int getCenteredX(int width);
int getCenteredY(int height);

StringArt getArtOfEntity(const Entity *entity);
StringArt getArtOfThing(enum AVAILABLE_ARTS artid, char **art, int length);
bool isActionMovement(Action action);

int getInnerMiddleWithOffset(int width, int divTimes, int indexToPick, int entityWidth);
Position getPositionWithInnerMiddleX(int width, int height, int divTimes, int indexToPick, int entityWidth);

bool areHideoutsClosed(const MapSkeleton *map);
Component getDefaultClockComponent(enum ClockType clockType);
int isEntityPositionHideoutValid(const Entity *entity, const MapSkeleton *map);

#endif // !FROGGER_SHORTCUTS_H
