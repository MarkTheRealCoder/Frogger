#ifndef FROGGER_SHORTCUTS_H
#define FROGGER_SHORTCUTS_H

#include "imports.h"
#include "../game/core.h"


// Il valore di terminatore di stringa. ("\0")
#define TERM 1


// Esegue lo sleep per un numero di microsecondi.
#define SLEEP_MICROS(quantity) usleep(quantity)
// Esegue lo sleep per un numero di millisecondi.
#define SLEEP_MILLIS(quantity) usleep(quantity * 1000)
// Esegue lo sleep per un numero di secondi.
#define SLEEP_SECONDS(quantity) sleep(quantity)


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

char *concat(const int n, ...);
bool str_eq(char *expected, char *toCompare);
char *str_packet_type(PacketType packetType);
char *str_direction(Direction direction);
char *str_coords(struct entity *entity);


int *get_screen_size();
void setScreenValidity(bool value);
bool isScreenValid();
int getCenteredX(int height);
int getCenteredY(int height);

StringArt getArt(struct entity *entity);

#endif // !FROGGER_SHORTCUTS_H

