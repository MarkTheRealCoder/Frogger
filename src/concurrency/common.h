#ifndef COMMON_H
#define COMMON_H

#include "../utils/imports.h"

// Il numero di processi / thread che possono venir creati.
#define MAX_CONCURRENCY 24

// Rappresenta il caso in cui una cella del buffer di comunicazione e' vuota.
#define COMMS_EMPTY -10000

#define MATCH_ID(id, message) ((message) >> 4) == 0 || ((message) >> 4) & (id)

#define AVAILABLE_DYNPID(result, ids)   \
{                                       \
    int copy = ids, count = 0;          \
                                        \
    while (copy & 1)                    \
    {                                   \
        count++;                        \
        copy = copy >> 1;               \
    }                                   \
                                        \
    result = 1 << (count);              \
    ids = ids | result;                 \
}

typedef enum {
    POLLING_NONE,
    POLLING_MANCHE_LOST,
    POLLING_GAME_PAUSE = 69,
    POLLING_FROG_DEAD
} PollingResult;

typedef enum {
    MESSAGE_RUN=3,
    MESSAGE_HALT=2,
    MESSAGE_STOP=1,
    MESSAGE_NONE=0
} SystemMessage;

typedef struct {
    int *rules;
    int buffer;
} ProductionRules;

typedef struct {
    unsigned int id;
    void *carriage;
    void (*producer)(void*);
} Packet;

PollingResult handle_clock(Component *component, int value);
PollingResult handle_entity(Component *component, int value, bool canPause);
PollingResult handle_entities(Component *component, int value);

#endif //COMMON_H
