#ifndef FROGGER_SHORTCUTS_H
#define FROGGER_SHORTCUTS_H

#include "imports.h"
#include "../game/core.h"


// malloc() - Alloca la memoria.
#define MALLOC(type, size) (type *)malloc(sizeof(type) * (size))

// calloc() - Alloca e azzera la memoria.
#define CALLOC(type, size) (type *)calloc((size), sizeof(type))

// realloc() - Rialloca la memoria.
#define REALLOC(type, arr, size) (type *)realloc(arr, sizeof(type) * (size))

// memcpy() - Copia la memoria da una parte a un'altra.
#define MEMCPY(from, to, type, size) memcpy(to, from, sizeof(type) * (size))

/*
 * Alloca la `packet->data`. 
 * Se `data` vive nello stack si deve eseguire una clonazione `clone = true`.
 * Se `data` vive nell'heap non e' necessario eseguire una clonazione `clone = false`.
 */
#define ALLOC_PACKET_DATA(from, to, type, size, clone)  \
    if (clone)                                          \
    {                                                   \
        to = MALLOC(type, size);                        \
        CRASH_IF_NULL(to)                               \
        MEMCPY(from, to, type, size);                   \
    }                                                   \
    else                                                \
        to = from;

/*
 * Crasha il programma se `pointer = NULL`.
 * Da utilizzare sempre dopo l'allocazione dinamica.
 */
#define CRASH_IF_NULL(pointer)                  \
    if (!pointer) {                             \
        perror("Cannot allocate memory!\n");    \
        exit(-1);                               \
    }

/*
 * Sleep function.
 */

typedef enum
{
    TIMEFRAME_MICROS,
    TIMEFRAME_MILLIS,
    TIMEFRAME_SECONDS
} TimeFrame;

void sleepy(int quantity, TimeFrame timeFrame);


/*
 * Number utils.
 */

int gen_num(int min, int max);
int choose_between(int count, ...);
int count_digits(int value);
int divide_if_possible(int dividend, int divisor);
double calculate_percentage(int dividend, int divisor);


/*
 * String utils.
 */

bool str_eq(char *expected, char *toCompare);
char *str_packet_type(PacketType packetType);
char *str_direction(Direction direction);

#endif // !FROGGER_SHORTCUTS_H

