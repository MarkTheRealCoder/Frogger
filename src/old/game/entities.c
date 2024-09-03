#include "entities.h"

Entity entities_default_frog()
{
    static int x = getCenteredX(CORE_GAME_ENTITY_SIZE);
    static int y = getCenteredY(CORE_GAME_ENTITY_SIZE) + 15;

    const Entity entity = {
        .current = getPosition(x, y),
        .last = getPosition(-1, -1),
        .type = ENTITY_TYPE__FROG,
        .trueType = TRUETYPE_FROG,
        .hps = TOTAL_LIVES,
        .width = FROG_WIDTH,
        .height = FROG_HEIGHT,
        .readyToShoot = false
    };

    return entity;
}

Entity entities_default_plant(int index)
{
    static int x = 0; // todo edit
    static int y = 0; // todo edit

    const Entity entity = {
        .current = getPosition(x, y),
        .last = getPosition(-1, -1),
        .type = ENTITY_TYPE__PLANT,
        .trueType = TRUETYPE_PLANT,
        .hps = 1,
        .width = PLANT_WIDTH,
        .height = PLANT_HEIGHT,
        .readyToShoot = false
    };

    return entity;
}

Entity new_entities_default_croc(int index)
{
    #define OFFSET 0

    static int x = 0; // todo edit
    static int y = 0; // todo edit

    const Entity entity = {
        .current = getPosition(x, y),
        .last = getPosition(-1, -1),
        .type = ENTITY_TYPE__CROC,
        .trueType = TRUETYPE_CROC,
        .hps = 1,
        .width = 0,
        .height = CORE_GAME_ENTITY_SIZE,
        .readyToShoot = false
    };

    return entity;
}

// todo redo completely
struct entity entities_default_croc(int *index)
{
    static int croc_y = 9; // todo edit for spacing
    static int in_row = 0;
    static int old_width = 0;
    static Action direction;
    
    int width = choose_between(2, CORE_GAME_CROCS_MIN_WIDTH, CORE_GAME_CROCS_MAX_WIDTH);
   
    if (in_row == 0)
    {
        direction = choose_between(2, ACTION_WEST, ACTION_EAST);
    }
    DEBUG("chosen direction: %s\n", str_direction(direction));
   
    int delay = choose_between(3, CORE_GAME_ENTITY_SIZE, CORE_GAME_ENTITY_SIZE * 2, CORE_GAME_ENTITY_SIZE * 3);
    DEBUG("chosen delay: %d\n", delay);

    bool is_west = direction == ACTION_WEST;
    
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
        croc_y += CORE_GAME_FROG_JUMP_Y;
    }

    DEBUG("GENERATED CROC -> x: %d, y: %d, width: %d, direction: %s\n", croc.x, croc.y, croc.width, str_direction(direction));

    return croc;
} // <- todo Redo



