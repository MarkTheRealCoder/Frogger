#ifndef FROGGER_ENTITIES_H
#define FROGGER_ENTITIES_H

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

#endif // !FROGGER_ENTITIES_H
