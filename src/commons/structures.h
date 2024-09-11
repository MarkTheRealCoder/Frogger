#ifndef STRUCTURES_H
#define STRUCTURES_H

// Il numero di processi / thread che possono venir creati.
#define MAX_CONCURRENCY 24

/**
 *  Qui ci sono tutte le strutture globalmente utili
*/
#pragma once

typedef enum {
    INNER_MESSAGE_NONE,
    INNER_MESSAGE_DESTROY_ENTITY,
    EVALUATION_MANCHE_WON,
    EVALUATION_MANCHE_LOST,
    EVALUATION_GAME_WON,
    EVALUATION_START_SECONDARY_CLOCK,
    POLLING_MANCHE_LOST,
    POLLING_GAME_PAUSE = 69,
    POLLING_FROG_DEAD
} InnerMessages;

typedef struct {
    int *rules;
    int buffer;
} ProductionRules;

typedef enum {
    ACTION_NORTH,
    ACTION_EAST,
    ACTION_SOUTH,
    ACTION_WEST,
    ACTION_SHOOT,
    ACTION_PAUSE = 69
} Action;

typedef enum {
    ENTITY_TYPE__EMPTY = 0,
    ENTITY_TYPE__FROG = 3,
    ENTITY_TYPE__CROC = 4,
    ENTITY_TYPE__PLANT = 11,
    ENTITY_TYPE__PROJECTILE = 35
} EntityType;

typedef enum entity_type {
    TRUETYPE_CROC           = 3,        // 0000-0000-0000-0011
    TRUETYPE_ANGRY_CROC     = 195,      // 0000-0000-1100-0011
    TRUETYPE_PLANT          = 2096,     // 0000-1000-0011-0000
    TRUETYPE_PLANT_HARMED   = 32752,    // 1000-0000-0011-0000
    TRUETYPE_FROG           = 16432,    // 0100-0000-0011-0000
    TRUETYPE_PROJ_FROG      = 8395,     // 0010-0000-1111-0011
    TRUETYPE_PROJ_PLANT     = 4144      // 0001-0000-0011-0000
} TrueType;

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    unsigned int length;
    char **art;
} StringArt;

typedef struct _string_node {
    struct _string_node *next;
    struct _string_node *prev;
    char *string;
    int color;
    int length;
} StringNode;

typedef struct {
    StringNode *last;
    int nodes;
} StringList;

typedef struct {
    Position current;
    Position last;
    EntityType type;
    TrueType trueType;
    int hps;
    int width;
    int height;
    short readyToShoot;
} Entity;

struct entities_list {
    Entity *e;
    struct entities_list *next;
};

typedef struct {
    struct entities_list *entities;
    int entity_num;
} Entities;

typedef struct {
    enum ClockType { CLOCK_MAIN, CLOCK_SECONDARY } type;
    unsigned int starting;
    int current;
    int fraction;
} Clock;

enum ComponentType { COMPONENT_NONE, COMPONENT_ENTITY, COMPONENT_ENTITIES, COMPONENT_CLOCK };
typedef struct {
    void *component;
    enum ComponentType type;
} Component;

typedef struct {
    Position *hideouts;
    Position sidewalk;
    Position river;
    Position garden;
    int width;
} MapSkeleton;

typedef struct {
    int lives;
    int score;
    Component components[MAX_CONCURRENCY];
    MapSkeleton map;
    StringList achievements;
} GameSkeleton;

typedef struct {
    unsigned int x;
    unsigned int y;
} Screen;

typedef struct generic_node {
    void *current;
    struct generic_node *next;
} GenericNode;

#endif
