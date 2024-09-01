#include "entities.h"
#include "../utils/shortcuts.h"

struct entity entities_default_frog(int *index)
{
    // todo: set a fixed x and y in the bottom part of the map
    int x = getCenteredX(CORE_GAME_ENTITY_SIZE);
    int y = getCenteredY(CORE_GAME_ENTITY_SIZE);

    struct entity frog = {
        .id = (*index)++,
        .type = ENTITY_TYPE__FROG,
        .width = CORE_GAME_ENTITY_SIZE,
        .alive = true,
        .x = x,
        .y = y,
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

/**
 * Ritorna l'altezza dell'entita' in base al suo tipo.
 * @param entityType Il tipo dell'entita'.
 */ 
int getHeightByEntityType(EntityType entityType) 
{
    return entityType == ENTITY_TYPE__PROJECTILE ? 1 : ENTITY_FROG_HEIGHT;
}

/**
 * Ritorna la priorita' dell'entita' in base al suo tipo.
 * @param entityType Il tipo dell'entita'.
 */
int getPriorityByEntityType(EntityType entityType) 
{
    int prio = 0; 

    switch (entityType) 
    {
        case ENTITY_TYPE__PLANT: 
            prio = 3;
            break;
        case ENTITY_TYPE__PROJECTILE: 
            prio = 4;
            break;
        case ENTITY_TYPE__CROC: 
            prio = 1;
            break;
        case ENTITY_TYPE__FROG: 
            prio = 2;
            break;
        case ENTITY_TYPE__EMPTY: 
            prio = 0;
            break;
    }

    return prio;
}

/**
 * Crea un cuboide a partire da una posizione, una larghezza e un'altezza.
 * @param leftcorner La posizione dell'angolo in basso a sinistra del cuboide.
 * @param width La larghezza del cuboide.
 * @param height L'altezza del cuboide.
 */
Cuboid createCuboid(Position leftcorner, int width, int height) 
{
    Cuboid cuboid = {
        .leftcorner = leftcorner,
        .rightcorner = {
            .x = leftcorner.x + width - 1,
            .y = leftcorner.y + height - 1
        }
    };

    return cuboid;
}

/**
 * Confronta due cuboidi e ritorna true se si sovrappongono, false altrimenti.
 * @param c1 Il primo cuboide.
 * @param c2 Il secondo cuboide.
 */
bool compareCuboids(Cuboid c1, Cuboid c2) 
{
    for (int x = c2.leftcorner.x; x <= c2.rightcorner.x; x++)
    {
        if (!(c1.leftcorner.x <= x && x <= c1.rightcorner.x))
        {
            continue;
        }

        for (int y = c2.leftcorner.y; y <= c2.rightcorner.y; y++) 
        {
            if (c1.leftcorner.y <= y && y <= c1.rightcorner.y) 
            {
                return true;
            }
        }
    }

    return false;
}

/**
 * Crea una posizione a partire da una struct entity.
 * @param e L'entita' da cui estrarre la posizione.
 */
Position getPositionFromEntity(struct entity e) 
{
    Position position = {
        .x = e.x,
        .y = e.y
    };

    return position;
}

/**
 * Crea un cuboide a partire da una struct entity.
 * @param e L'entita' da cui estrarre il cuboide.
 */
Cuboid getCuboidFromEntity(struct entity e) 
{
    return createCuboid(getPositionFromEntity(e), e.width, getHeightByEntityType(e.type));
}

/**
 * Controlla se due entita' stanno collidendo.
 * @param e1 La prima entita'.
 * @param e2 La seconda entita'.
 */
CollisionPacket areColliding(struct entity e1, struct entity e2) 
{
    CollisionPacket collisionPacket = {
        .e1 = e1.type,
        .e2 = e2.type,
        .e1_priority = getPriorityByEntityType(e1.type),
        .e2_priority = getPriorityByEntityType(e2.type)
    };

    Cuboid c1 = getCuboidFromEntity(e1);
    Cuboid c2 = getCuboidFromEntity(e2);

    if (!compareCuboids(c1, c2)) 
    {
        collisionPacket.collision_type = COLLISION_AVOIDED;
    }
    else 
    {
        collisionPacket.collision_type = ((e1.type & e2.type) == 3) ? COLLISION_DAMAGING : COLLISION_OVERLAPPING;
    }

    return collisionPacket;
}

