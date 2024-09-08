#ifndef COMMON_H
#define COMMON_H

#include "../../commons/imports.h"
#include "../../commons/structures.h"

#pragma once
// Rappresenta il caso in cui una cella del buffer di comunicazione e' vuota.
#define COMMS_EMPTY -10

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
    MESSAGE_RUN=3,
    MESSAGE_HALT=2,
    MESSAGE_STOP=1,
    MESSAGE_NONE=0
} SystemMessage;

enum LFI {
    LFI_EMPTY_ID = 0,
    LFI_SPECIFIC_ID
};

typedef struct {
    unsigned int id;
    void *carriage;
    void (*producer)(void*);
    int ms;
} Packet;

PollingResult handle_clock(Component *component, int value);
PollingResult handle_entity(Component *component, int value, int canPause);
PollingResult handle_entities(Component *component, int value);

Component *find_component(int index, GameSkeleton *game);
Component **find_components(GameSkeleton *game, ...);
void update_position (Entity *e, Action movement);
void user_listener(void *_rules);
void entity_move(void *_rules);
void timer_counter(void *_rules);
SystemMessage create_message(const SystemMessage action, const int receivers);
#endif //COMMON_H
