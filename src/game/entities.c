#include "entities.h"
#include "../utils/shortcuts.h"

struct entity entities_default_frog(int *index)
{
    // todo: set a fixed x and y in the bottom part of the map

    struct entity frog = {
        .id = (*index)++,
        .type = ENTITY_TYPE__FROG,
        .width = CORE_GAME_ENTITY_SIZE,
        .alive = true,
        .x = 0,
        .y = 0,
        .direction = DIRECTION_NORTH
    };

    return frog;
}

struct entity entities_default_plant(int *index)
{
    static int plant_x = 0;

    struct entity plant = {
        .id = (*index)++,
        .type = ENTITY_TYPE__PLANT,
        .width = CORE_GAME_ENTITY_SIZE,
        .alive = true,
        .x = plant_x,
        .y = 0,
        .direction = DIRECTION_SOUTH
    };

    plant_x += 9; // todo edit for spacing

    return plant;
}

struct entity entities_default_croc(int *index)
{
    static int croc_y = 9; // todo edit for spacing
    static int in_row = 0;
    static int old_width = 0;
    static Direction direction;
    
    int width = choose_between(2, CORE_GAME_CROCS_MIN_WIDTH, CORE_GAME_CROCS_MAX_WIDTH);
   
    if (in_row == 0)
    {
        direction = choose_between(2, DIRECTION_WEST, DIRECTION_EAST);
    }
    DEBUG("chosen direction: %s\n", str_direction(direction));
   
    int delay = choose_between(3, CORE_GAME_ENTITY_SIZE, CORE_GAME_ENTITY_SIZE * 2, CORE_GAME_ENTITY_SIZE * 3);
    DEBUG("chosen delay: %d\n", delay);

    bool is_west = direction == DIRECTION_WEST;
    
    int croc_x = is_west ? CORE_GAME_MAP_WIDTH : 0;
    DEBUG("first croc_x: %d\n", croc_x);

    croc_x += is_west ? width + delay : -(width + delay);
    DEBUG("second croc_x: %d\n", croc_x);

    croc_x += is_west ? old_width : -old_width;
    DEBUG("third croc_x: %d\n", croc_x);

    struct entity croc = {
        .id = (*index)++,
        .type = ENTITY_TYPE__CROC,
        .width = width,
        .alive = true,
        .x = croc_x,
        .y = croc_y,
        .direction = direction 
    };

    old_width = width;

    if (++in_row == 2)
    {
        in_row = 0;
        old_width = 0;
        croc_y += CORE_GAME_FROG_JUMP;
    }

    DEBUG("GENERATED CROC -> x: %d, y: %d, width: %d, direction: %s\n", croc.x, croc.y, croc.width, str_direction(direction));

    return croc;
}


CollisionPacket areColliding(struct entity e1, struct entity e2) {
    
}

