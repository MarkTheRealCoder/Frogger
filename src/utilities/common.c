#include "common.h"

Entity entities_default_frog()
{
    //int x = getInnerMiddleWithOffset(mapSkeleton.width, 1, 1, GAME_ENTITY_SIZE);
    //int y = getCenteredY(GAME_ENTITY_SIZE) + 15; // todo edit

    //Position startPosition = getPosition(x, y);

    Entity entity = {
        .current = getPosition(0, 0),
        .last = getPosition(-1, -1),
        .type = ENTITY_TYPE__FROG,
        .trueType = TRUETYPE_FROG,
        .hps = 2,
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
    //#define OFFSET 0

    //static int x = 0; // todo edit
    //static int y = 0; // todo edit

    Entity entity = {
        .current = getPosition(0, 0),
        .last = getPosition(-1, -1),
        .type = ENTITY_TYPE__CROC,
        .trueType = TRUETYPE_CROC,
        .hps = 1,
        .width = 0,
        .height = GAME_ENTITY_SIZE,
        .readyToShoot = false
    };

    return entity;
}

Entity create_projectile(Entity *master) {
    bool masterIsFrog = master->type == ENTITY_TYPE__FROG;
    Position p = master->current;
    p.x += 1;
    p.y += (masterIsFrog) ? -1 : FROG_HEIGHT;
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
    c->current = c->starting = value * 1000;
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
            entities[0] = e;
            entities[1] = NULL;
        }
        break;
        case COMPONENT_ENTITIES:
        {
            Entities *e = (Entities*) c.component;
            entities = CALLOC(void*, e->entity_num + 1);
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

struct entities_list *create_default_entities(GameSkeleton *game, int loadFromSkeleton)
{
    struct entities_list *entities = NULL;
    int index = 0;

    while (true)
    {
        struct entities_list *node = CALLOC(struct entities_list, 1);
        CRASH_IF_NULL(node)

        if (loadFromSkeleton)
        {
            if (index == MAX_CONCURRENCY)
            {
                free(node);
                break;
            }

            void **binded = getEntitiesFromComponent(game->components[index]);

            index++;

            if (!binded) {
                free(node);
                continue;
            }

            int inner_index = 0;
            while (binded[inner_index]) {
                if (inner_index) {
                    struct entities_list *inner_node = CALLOC(struct entities_list, 1);
                    inner_node->next = node;
                    node = inner_node;
                }
                node->e = binded[inner_index];
                if (!inner_index) node->next = entities;
                inner_index++;
            }
            free(binded);
        }
        else {
            if (index == COMPONENT_CLOCK_INDEX) {
                free(node);
                break;
            }

            if (index < COMPONENT_CLOCK_INDEX)
            {
                node->e = CALLOC(Entity, 1);
                CRASH_IF_NULL(node->e)
            }

            if (index == COMPONENT_FROG_INDEX)          *(node->e) = entities_default_frog();
            else if (index <= COMPONENT_CROC_INDEXES)   *(node->e) = new_entities_default_croc();
            else if (index < COMPONENT_CLOCK_INDEX)     *(node->e) = entities_default_plant();

            game->components[index] = (Component){.type=COMPONENT_ENTITY, .component=node->e};
            node->next = entities;
            index++;
        }
        entities = node;
    }

    return entities;
}

void create_new_entities(struct entities_list **list, Component components[MAX_CONCURRENCY]) {
    struct entities_list *entity = *list;
    Position p = {0, 0};
    while (entity) {
        Entity *e = entity->e;
        if (e->readyToShoot) {
            Entity *new = CALLOC(Entity, 1);
            CRASH_IF_NULL(new);
            *new = create_projectile(e);
            // Adding entity to the main list
            struct entities_list *node = CALLOC(struct entities_list, 1);
            node->next = *list;
            node->e = new;
            *list = node;
            // Adding entity to its component's list
            node = CALLOC(struct entities_list, 1);
            int index = (e->type == ENTITY_TYPE__FROG) ? COMPONENT_FROG_PROJECTILES_INDEX : COMPONENT_PROJECTILES_INDEX;
            Entities *es = (Entities *) components[index].component;
            es->entity_num++;
            node->next = es->entities;
            node->e = new;
            es->entities = node;
        }
        entity = entity->next;
    }
}