#include "common.h"

Entity entities_default_frog(MapSkeleton mapSkeleton)
{
    int x = getInnerMiddleWithOffset(mapSkeleton.width, 1, 1, CORE_GAME_ENTITY_SIZE);
    int y = getCenteredY(CORE_GAME_ENTITY_SIZE) + 15; // todo edit

    Position startPosition = getPosition(x, y);

    Entity entity = {
        .current = startPosition,
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

Entity entities_default_plant(MapSkeleton mapSkeleton, int index)
{
    int x = getInnerMiddleWithOffset(mapSkeleton.width, 2, index + 1, PLANT_WIDTH);
    int y = getCenteredY(CORE_GAME_ENTITY_SIZE) - 12; // todo edit

    Position startPosition = getPosition(x, y);

    Entity entity = {
        .current = startPosition,
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

Entity new_entities_default_croc(int index)
{
    #define OFFSET 0

    static int x = 0; // todo edit
    static int y = 0; // todo edit

    Entity entity = {
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
        if (loadFromSkeleton) {

            if (index == MAX_CONCURRENCY) {
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
            // todo edit
        }
        entities = node;
    }

    return entities;
}