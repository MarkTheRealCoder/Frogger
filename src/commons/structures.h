#ifndef STRUCTURES_H
#define STRUCTURES_H

// Il numero di processi / thread che possono venir creati.
#define MAX_CONCURRENCY 24

/**
 *  Qui ci sono tutte le strutture globalmente utili
*/
#pragma once

typedef enum {
    POLLING_NONE,
    POLLING_MANCHE_LOST,
    POLLING_GAME_PAUSE = 69,
    POLLING_FROG_DEAD
} PollingResult;

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
    TRUETYPE_CROC,
    TRUETYPE_ANGRY_CROC,
    TRUETYPE_PLANT,
    TRUETYPE_PLANT_HARMED,
    TRUETYPE_FROG,
    TRUETYPE_FROG_HARMED,
    TRUETYPE_PROJ_FROG,
    TRUETYPE_PROJ_PLANT
} TrueType;

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    unsigned int length;
    char **art;
} StringArt;

typedef struct _string_node{
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
    enum {CLOCK_MAIN, CLOCK_SECONDARY} type;
    unsigned int starting;
    int current;
    int fraction;
} Clock;

typedef struct {
    void *component;
    enum {COMPONENT_ENTITY, COMPONENT_ENTITIES, COMPONENT_CLOCK} type;
} Component;

typedef struct {
    Position *hideouts;
    Position sidewalk;
    Position river;
    Position garden;
    int width;
} MapSkeleton;

typedef struct {
    int current_plants;
    int current_projectiles;
    int current_frog_projectiles;
    Component components[MAX_CONCURRENCY];
    MapSkeleton map;
    StringList achievements;
} GameSkeleton;

#endif


