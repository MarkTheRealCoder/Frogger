#ifndef COMMON_H
#define COMMON_H

#include "../../commons/imports.h"
#include "../../commons/structures.h"
#include "../../commons/drawing.h"

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

typedef struct _timer_t{
    struct timeval start;
    unsigned int id;
    struct _timer_t *next;
} Timer;

typedef enum {
    MESSAGE_RUN = 3,
    MESSAGE_HALT = 2,
    MESSAGE_STOP = 1,
    MESSAGE_NONE = 0
} SystemMessage;

typedef enum {
    VALIDATION_NONE,
    VALIDATION_BREAK
} ValidationResult;

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

InnerMessages handle_clock(Component *component, int *value);
InnerMessages handle_entity(Component *component, int value, int canPause);
InnerMessages handle_entities(Component *component, int value);
void invalidate_entity(Entity *e);
void handle_invalid_entities(struct entities_list **list, Component components[MAX_CONCURRENCY]);
void reset_secondary_timer(int *buffer, GameSkeleton *game);

Component *find_component(int index, GameSkeleton *game);
Component **find_components(GameSkeleton *game, ...);
void update_position (Entity *e, Action movement);
void user_listener(void *_rules);
void entity_move(void *_rules);
void timer_counter(void *_rules);
SystemMessage create_message(SystemMessage action, int receivers);
Action getDefaultActionByY(MapSkeleton map, int y, bool reset);

InnerMessages apply_validation(GameSkeleton *game, struct entities_list **list);
InnerMessages apply_physics(GameSkeleton *game, struct entities_list **list);

Position set_croc_position(MapSkeleton map, int y, int padding);
Position reset_croc_position(MapSkeleton map, int y);
int *reset_game(GameSkeleton *game, struct entities_list **list);

void free_memory(GameSkeleton *game, struct entities_list **list);

void reset_moved(struct entities_list *list);

void clear_timers();
int destroy_timer(unsigned int index);
void update_timer(unsigned int index);
int add_timer(unsigned int index);
int time_elapsed(unsigned int index);

void gen_plants(GameSkeleton *game);

#endif //COMMON_H
