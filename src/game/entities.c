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

/**
 * Crea una posizione basandosi sulle coordinate.
 * @param x La coordinata X.
 * @param y La coordinata Y.
 * @return La posizione delle coordinate.
 */
Position getPosition(const int x, const int y)
{
    return (Position) { x, y };
}

/**
 * Ritorna l'altezza dell'entita' in base al suo tipo.
 * @param entityType Il tipo dell'entita'.
 */ 
int getHeightByEntityType(const EntityType entityType)
{
    return entityType == ENTITY_TYPE__PROJECTILE ? 1 : ENTITY_FROG_HEIGHT;
}

/**
 * Ritorna la priorita' dell'entita' in base al suo tipo.
 * @param entityType Il tipo dell'entita'.
 */
int getPriorityByEntityType(const EntityType entityType)
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
 * @param leftCorner La posizione dell'angolo in basso a sinistra del cuboide.
 * @param width La larghezza del cuboide.
 * @param height L'altezza del cuboide.
 */
Cuboid createCuboid(const Position leftCorner, const int width, const int height)
{
    const Cuboid cuboid = {
        .leftcorner = leftCorner,
        .rightcorner = {
            .x = leftCorner.x + width - 1,
            .y = leftCorner.y + height - 1
        }
    };

    return cuboid;
}

/**
 * Confronta due cuboidi e ritorna true se si sovrappongono, false altrimenti.
 * @param c1 Il primo cuboide.
 * @param c2 Il secondo cuboide.
 */
bool compareCuboids(const Cuboid c1, const Cuboid c2)
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
 * Crea un cuboide a partire da una struct entity.
 * @param e L'entita' da cui estrarre il cuboide.
 */
Cuboid getCuboidFromEntity(const struct entity e)
{
    return createCuboid(getPositionFromEntity(e), e.width, getHeightByEntityType(e.type));
}

/**
 * Controlla se due entita' stanno collidendo.
 * @param e1 La prima entita'.
 * @param e2 La seconda entita'.
 */
CollisionPacket areColliding(const struct entity e1, const struct entity e2)
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
        collisionPacket.collision_type = ((e1.type & 3) == 3 && (e2.type & 3) == 3) ? COLLISION_DAMAGING : COLLISION_OVERLAPPING;
    }

    return collisionPacket;
}

