#ifndef FROGGER_ENTITIES_H
#define FROGGER_ENTITIES_H

#include "../utils/imports.h"
#include "../graphics/drawing.h"

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
    DIRECTION_NORTH,
    DIRECTION_SOUTH,
    DIRECTION_EAST,
    DIRECTION_WEST
} Direction;

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
    EntityType type;

    unsigned int width;

    bool alive;
    int x;
    int y;
    Direction direction;
};

struct entity_node {
    struct entity entity;
    struct entity_node *next;
};

struct croc {
    entity_id_t id;
    bool angry;
    bool dipped;
};

struct plant {
    entity_id_t id;
    // int lives; optional
};

struct projectile {
    entity_id_t id;
    entity_id_t shooter;
    //todo many attributes if we want to implement different types of projectiles.
};

struct frog {
    entity_id_t id;
    int lives;
    //todo let's see later what we can add...
};

// suggestion: add a field to the entity struct to bind a display function for the specific entity.

typedef struct {
    Position leftcorner;
    Position rightcorner;
} Cuboid;

typedef struct {
    EntityType e1;
    EntityType e2;
    int e1_priority;
    int e2_priority;
    enum {COLLISION_OVERLAPPING, COLLISION_DAMAGING, COLLISION_AVOIDED} collision_type;
} CollisionPacket;


struct entity entities_default_frog(int *index);
struct entity entities_default_plant(int *index);
struct entity entities_default_croc(int *index);
CollisionPacket areColliding(struct entity e1, struct entity e2);

#endif // !FROGGER_ENTITIES_H
