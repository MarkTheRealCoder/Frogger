#ifndef FROGGER_ENTITIES_H
#define FROGGER_ENTITIES_H

#define GLOBAL_HEIGHT 3


typedef enum {
    DIRECTION_NORTH,
    DIRECTION_SOUTH,
    DIRECTION_EAST,
    DIRECTION_WEST
} Direction;

typedef enum {
    ENTITY_TYPE__FROG,
    ENTITY_TYPE__CROC,
    ENTITY_TYPE__PLANT,
    ENTITY_TYPE__PROJECTILE
} EntityType;

typedef unsigned long long entity_id_t;

struct entity {
    entity_id_t id;
    EntityType entity_type;

    unsigned int width;

    bool alive;
    int x;
    int y;
    Direction direction;
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

#endif // !FROGGER_ENTITIES_H
