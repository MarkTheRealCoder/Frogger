#ifndef FROGGER_ENTITIES_H
#define FROGGER_ENTITIES_H

#include "../utils/imports.h"

#define ENTITIES_FROG_ID 0

#define ENTITIES_CROC_ID_START 1
#define ENTITIES_CROC_ID_END 16

#define ENTITIES_PLANT_ID_START 17
#define ENTITIES_PLANT_ID_END 20

#define ENTITIES_PROJECTILE_PLANT_ID_START 21
#define ENTITIES_PROJECTILE_PLANT_ID_END 24

#define ENTITY_FROG_HEIGHT 3
#define ENTITY_FROG_WIDTH 3

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

typedef int entity_id_t;

struct entity {
    entity_id_t id;

    EntityType type; // rappresenta il tipo generico dell'entità
    enum entity_type trueType; // rappresenta il tipo specifico dell'entità

    unsigned int width;

    bool alive;
    int x;
    int y;
    Action direction;
};

struct entity_node {
    struct entity entity;
    struct entity_node *next;
};

typedef struct {
    Position leftcorner;
    Position rightcorner;
} Cuboid;

typedef struct {
    EntityType e1;
    EntityType e2;

    int e1_priority;
    int e2_priority;

    enum {
        COLLISION_OVERLAPPING, 
        COLLISION_DAMAGING,
        COLLISION_AVOIDED
    } collision_type;
} CollisionPacket;

Position getPosition(int x, int y);

Entity entities_default_frog();
Entity entities_default_plant(int index);
Entity entities_default_croc(int index);

Position getPositionFromEntity(struct entity e);
int getPriorityByEntityType(EntityType entityType);
Cuboid getCuboidFromEntity(struct entity e);
CollisionPacket areColliding(struct entity e1, struct entity e2);

#endif // !FROGGER_ENTITIES_H
