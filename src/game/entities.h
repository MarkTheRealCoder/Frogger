#ifndef ENTITIES_H
#define ENTITIES_H

#define GLOBAL_HEIGHT 3

typedef char byte;
typedef unsigned long long entity_id_t;

typedef enum {
    DIRECTION_NORTH,
    DIRECTION_SOUTH,
    DIRECTION_EAST,
    DIRECTION_WEST
} Direction;

struct entity {
    unsigned int width;
    entity_id_t id;
    Direction direction;
    bool alive;
    char *shape[GLOBAL_HEIGHT];  
};

/* 
FROG
        \-/                 
        (_)                 
        / \   

CROP
        \|/
        -o-
        /|\

PROJECTILE
         |              
*/

struct croc {
    entity_id_t id;
    bool angry;
    bool dipped;
};

struct crop {
    entity_id_t id;
    byte lives; // from 1 to 3
};

struct projectile {
    entity_id_t id;
    entity_id_t shooter;
    //todo many attributes if we want to implement different types of projectiles.
};

struct frog {
    entity_id_t id;
    byte lives;
    byte hps;
    //todo let's see later what we can add...
};

// suggestion: add a field to the entity struct to bind a display function for the specific entity.

#endif