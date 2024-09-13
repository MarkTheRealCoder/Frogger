#include "common.h"

Entity entities_default_frog(MapSkeleton map)
{
    Entity entity = {
        .current = getPositionWithInnerMiddleX(map.width, map.sidewalk.y, 1, 1, FROG_WIDTH),
        .last = getPosition(-1, -1),
        .type = ENTITY_TYPE__FROG,
        .trueType = TRUETYPE_FROG,
        .hps = 1,
        .width = FROG_WIDTH,
        .height = FROG_HEIGHT,
        .readyToShoot = false
    };

    return entity;
}

Entity entities_default_plant()
{
    //int x = getInnerMiddleWithOffset(mapSkeleton.width, 2, index + 1, PLANT_WIDTH);
    //int y = getCenteredY(GAME_ENTITY_SIZE) - 12; // todo edit

    //Position startPosition = getPosition(x, y);

    Entity entity = {
        .current = getPosition(0, 0),
        .last = getPosition(-1, -1),
        .type = ENTITY_TYPE__PLANT,
        .trueType = TRUETYPE_PLANT,
        .hps = 2,
        .width = PLANT_WIDTH,
        .height = PLANT_HEIGHT,
        .readyToShoot = false
    };

    return entity;
}

Entity new_entities_default_croc()
{
    Entity entity = {
        .current = getPosition(0, 0),
        .last = getPosition(-1, -1),
        .type = ENTITY_TYPE__CROC,
        .trueType = choose_between(4, TRUETYPE_ANGRY_CROC, TRUETYPE_CROC, TRUETYPE_CROC, TRUETYPE_CROC),
        .hps = 1,
        .width = GAME_CROCS_MAX_WIDTH, // choose_between(2, GAME_CROCS_MIN_WIDTH, GAME_CROCS_MAX_WIDTH)
        .height = GAME_ENTITY_SIZE,
        .readyToShoot = false
    };

    return entity;
}

Entity create_projectile(Entity *master, MapSkeleton map)
{
    bool masterIsFrog = master->type == ENTITY_TYPE__FROG;
    Position p = master->current;
    p.x += 1;
    p.y += (masterIsFrog) ? -1 : FROG_HEIGHT;

    if (WITHIN_BOUNDARIES(p.x, p.y, map) && p.y <= map.garden.y)
    {
        p = getPosition(-1, -1);
    }

    return (Entity) {
            .current = p,
            .last = getPosition(-1, -1),
            .readyToShoot = false,
            .type = ENTITY_TYPE__PROJECTILE,
            .trueType = (masterIsFrog) ? TRUETYPE_PROJ_FROG : TRUETYPE_PROJ_PLANT,
            .hps = 1,
            .width = 1,
            .height = 1
    };
}

Clock *create_clock(unsigned int value, enum ClockType type)
{
    Clock *c = CALLOC(Clock, 1);
    CRASH_IF_NULL(c)

    c->type = type;
    c->current = c->starting = value * 1000 * 100;
    c->fraction = (int) (c->starting / CLOCK_DISPLAY_SIZE);

    return c;
}

Entities *create_entities_group()
{
    Entities *e = CALLOC(Entities, 1);
    CRASH_IF_NULL(e)

    e->entity_num = 0;
    e->entities = NULL;

    return e;
}

void **getEntitiesFromComponent(Component c)
{
    void **entities = NULL;

    switch (c.type)
    {
        case COMPONENT_ENTITY:
        {
            Entity *e = (Entity*) c.component;
            entities = CALLOC(void*, 2);
            CRASH_IF_NULL(entities)
            entities[0] = e;
            entities[1] = NULL;
        }
        break;
        case COMPONENT_ENTITIES:
        {
            Entities *e = (Entities*) c.component;
            entities = CALLOC(void*, e->entity_num + 1);
            CRASH_IF_NULL(entities)
            entities[e->entity_num] = NULL;
            struct entities_list *en = e->entities;

            for (int i = 0; i < e->entity_num; i++)
            {
                entities[i] = en->e;
                en = en->next;
            }
        }
        break;
    }

    return entities;
}

struct entities_list *create_default_entities(GameSkeleton *game)
{
    struct entities_list *entities = NULL;
    int index = 0;

    while (true)
    {
        struct entities_list *node = CALLOC(struct entities_list, 1);
        CRASH_IF_NULL(node)

        if (index == COMPONENT_CLOCK_INDEX) {
            free(node);
            break;
        }

        if (index < COMPONENT_CLOCK_INDEX)
        {
            node->e = CALLOC(Entity, 1);
            CRASH_IF_NULL(node->e)
        }

        if (index == COMPONENT_FROG_INDEX)          *(node->e) = entities_default_frog(game->map);
        else if (index <= COMPONENT_CROC_INDEXES)   *(node->e) = new_entities_default_croc();
        else if (index < COMPONENT_CLOCK_INDEX)     *(node->e) = entities_default_plant();

        game->components[index] = (Component){.type=COMPONENT_ENTITY, .component=node->e};
        node->next = entities;
        index++;

        entities = node;
    }

    return entities;
}

void create_new_entities(struct entities_list **list, Component components[MAX_CONCURRENCY], MapSkeleton map) {
    struct entities_list *entity = *list;
    Position p = {0, 0};
    while (entity) {
        Entity *e = entity->e;
        if (e->readyToShoot) {
            int index = (e->type == ENTITY_TYPE__FROG) ? COMPONENT_FROG_PROJECTILES_INDEX : COMPONENT_PROJECTILES_INDEX;
            Entities *es = (Entities *) components[index].component;
            if (es->entity_num <= 2) {
                Entity *new = CALLOC(Entity, 1);
                CRASH_IF_NULL(new);
                *new = create_projectile(e, map);
                if (new->current.x == -1 && new->current.y == -1) {
                    free(new);
                    return;
                }
                // Adding entity to the main list
                struct entities_list *node = CALLOC(struct entities_list, 1);
                CRASH_IF_NULL(node)
                node->next = *list;
                node->e = new;
                *list = node;
                // Adding entity to its component's list
                node = CALLOC(struct entities_list, 1);
                CRASH_IF_NULL(node)
                es->entity_num++;
                node->next = es->entities;
                node->e = new;
                es->entities = node;
            }
            e->readyToShoot = false;
        }
        entity = entity->next;
    }
}