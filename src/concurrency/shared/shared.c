#include "shared.h"

InnerMessages handle_clock(Component *component, int value)
{
    Clock *clock = (Clock*) component->component;
    clock->current = value;

    if (clock->type == CLOCK_MAIN && value <= 0)
    {
        return POLLING_MANCHE_LOST;
    }

    if (clock->type == CLOCK_SECONDARY && value <= 0)
    {
        // todo: must be checked by collision checker
        return POLLING_FROG_DEAD;
    }

    return INNER_MESSAGE_NONE;
}

InnerMessages handle_entity(Component *component, int value, int canPause)
{
    Entity *entity = (Entity *) component->component;

    if (isActionMovement(value))
    {
        update_position(entity, value);
    }
    else if (value == ACTION_SHOOT)
    {
        entity->readyToShoot = true;
    }
    else if (canPause)
    {
        return POLLING_GAME_PAUSE;
    }

    return INNER_MESSAGE_NONE;
}

InnerMessages handle_entities(Component *component, int value)
{
    Entities *entities = (Entities *) component->component;
    struct entities_list *list = entities->entities;

    while (list != NULL)
    {
        Component entityComponent = {
            .component = list->e,
            .type = COMPONENT_ENTITY
        };

        handle_entity(&entityComponent, value, false);
        list = list->next;
    }

    return INNER_MESSAGE_NONE;
}

Component *find_component(const int index, GameSkeleton *game)
{
    return &game->components[index];
}

void update_position(Entity *e, const Action action)
{
    if (!isActionMovement(action))
    {
        return;
    }

    e->last = e->current;

    if (action == ACTION_WEST || action == ACTION_EAST)
    {
        e->current.x += (action == ACTION_WEST) ? -GAME_FROG_JUMP_X : GAME_FROG_JUMP_X;
    }
    else
    {
        if (e->type == ENTITY_TYPE__FROG)
        {
            e->current.y += (action == ACTION_NORTH) ? -GAME_FROG_JUMP_Y : GAME_FROG_JUMP_Y;
        }
        else
        {
            e->current.y += (action == ACTION_NORTH) ? -GAME_FROG_JUMP_X : GAME_FROG_JUMP_X;
        }
    }
}

void user_listener(void *_rules)
{
    ProductionRules *rules = (ProductionRules *) _rules;
    int value = -1;

    do
    {
        switch(wgetch(stdscr))
        {
            case 'W':
            case 'w':
            case KEY_UP:
                value = ACTION_NORTH;
                break;
            case 'S':
            case 's':
            case KEY_DOWN:
                value = ACTION_SOUTH;
                break;
            case 'A':
            case 'a':
            case KEY_LEFT:
                value = ACTION_WEST;
                break;
            case 'D':
            case 'd':
            case KEY_RIGHT:
                value = ACTION_EAST;
                break;
            case 'P':
            case 'p':
                value = ACTION_PAUSE;
                break;
            case ' ':
                value = ACTION_SHOOT;
                break;
            default:
                break;
        }
    } while (value == -1);

    rules->buffer = value;
}

void entity_move(void *_rules)
{
    ProductionRules *rules = (ProductionRules*)_rules;
    int value = rules->rules[0];
    rules->buffer = value;
}

void timer_counter(void *_rules)
{
    ProductionRules *rules = (ProductionRules*)_rules;
    int value = (int)((int*)rules->rules)[0]; // current value (updated in main routine)
    int part = (int)((int*)rules->rules)[1]; // fraction to be subtracted from value;
    rules->buffer = value - part;
}

/**
 * Crea un messaggio da inviare.
 * @param action        L'azione.
 * @param receivers     I riceventi.
 * @return              Il messaggio da inviare.
 */
SystemMessage create_message(const SystemMessage action, const int receivers)
{
    return action + (receivers << 4);
}

Action getDefaultActionByY(MapSkeleton map, int y, bool reset)
{
    static int RANDOMIC_SEED = -1;

    if (reset)
    {
        RANDOMIC_SEED = gen_num(0, 1);
    }

    return (((y - map.river.y) + RANDOMIC_SEED) % 2 == 0) ? ACTION_EAST : ACTION_WEST;
}

Position set_croc_position(MapSkeleton map, int y, int padding)
{
    int x = (getDefaultActionByY(map, y, false) == ACTION_WEST) ? map.sidewalk.x + map.width + padding : map.sidewalk.x - padding;
    return getPosition(x, y);
}

Position reset_croc_position(MapSkeleton map, int y)
{
    return set_croc_position(map, y, 1);
}

Position invalidate_position(Entity *e, struct entities_list *list)
{
    Position lastEntityPos = e->last;
    static int pos = 0;
    while (list)
    {
        Entity *comparison = list->e;
        int x = comparison->current.x, y = comparison->current.y;
        int width = comparison->width, height = comparison->height;

        if (e != comparison)
        {
            bool collisionInRangeX = x <= lastEntityPos.x && lastEntityPos.x < x + width;
            bool collisionInRangeY = y <= lastEntityPos.y && lastEntityPos.y < y + height;

            if (collisionInRangeX && collisionInRangeY && (lastEntityPos.x != 0 && lastEntityPos.y != 0))
            {
                //display_debug_string(24 + (pos++ % 3), "POSITION INVALIDATED FOR %i BY %i (THEY ARE DIFFERENT %i)", 80, e->trueType, comparison->trueType, e != comparison);
                return getPosition(0, 0);
            }
        }
        list = list->next;
    }

    return lastEntityPos;
}

void remove_entity_from_list(struct entities_list **list, Entity *e) {
    struct entities_list *pivot = *list;
    struct entities_list *prev = NULL;
    bool isFirst = true;
    while (pivot) {
        if (pivot->e == e) break;
        prev = pivot;
        pivot = pivot->next;
    }

    if (!pivot) return;

    if (prev) prev->next = pivot->next;
    else *list = pivot->next;

    free(pivot);
}

void destroy_entity(struct entities_list **el, struct entities_list **list, Entity *e) {
    remove_entity_from_list(el, e);
    remove_entity_from_list(list, e);
    free(e);
}

InnerMessages validate_entity(Entity *entity, const MapSkeleton  *map, struct entities_list **list)
{
    int *x = &entity->current.x;
    int *y = &entity->current.y;

    int *lastX = &entity->last.x;
    int *lastY = &entity->last.y;

    switch (entity->type)
    {
        case ENTITY_TYPE__FROG:
        {
            bool notWithinBoundaries = !WITHIN_BOUNDARIES(*x, *y, *map) || !WITHIN_BOUNDARIES(*x + 2, *y, *map);
            int entityInsideOfHideout = isEntityPositionHideoutValid(entity, map);

            //display_debug_string(1, "FROG: nw=%d, h=%d sidewalk.y: %i - Y: %i", 40, notWithinBoundaries, entityInsideOfHideout, map->sidewalk.y, *y);
            //display_debug_string(2, "LY: %i - FY: %i", 40, map->sidewalk.y, *y);

            if (notWithinBoundaries || !entityInsideOfHideout)
            {
                //display_debug_string(5, "PROVA", 40);
                *x = *lastX;
                *y = *lastY;
            }
            else
            {
                entity->last = invalidate_position(entity, *list);
            }

            if (entityInsideOfHideout >= 2)
            {
                map->hideouts[entityInsideOfHideout - 2].x = 0;
                map->hideouts[entityInsideOfHideout - 2].y = 0;

                return EVALUATION_MANCHE_WON;
            }
        } break;
        case ENTITY_TYPE__CROC:
        {
            if (!WITHIN_BOUNDARIES(*x, *y, (*map)))
            {
                bool isActionWest = getDefaultActionByY(*map, *y, false) == ACTION_WEST;
                bool invalid = (isActionWest && *x + entity->width < map->sidewalk.x) || (!isActionWest && *x > map->sidewalk.x + map->width);

                if (invalid)
                {
                    entity->current = reset_croc_position(*map, *y);
                }
            }
        } break;
        case ENTITY_TYPE__PLANT:
        {
            if (!WITHIN_BOUNDARIES(*x, *y, (*map)))
            {
                entity->readyToShoot = false;
            }
        } break;
        case ENTITY_TYPE__PROJECTILE:
        {
            if (!WITHIN_BOUNDARIES(*x, *y, (*map)) || *y < map->garden.y)
            {
                return INNER_MESSAGE_DESTROY_ENTITY;
            }
            else
            {
                entity->last = invalidate_position(entity, *list);
            }
        } break;
        default:
            break;
    }

    return INNER_MESSAGE_NONE;
}

InnerMessages apply_validation(GameSkeleton *game, struct entities_list **list) {

    // display_debug_string("APPLY VALIDATION", 40);

    InnerMessages result = INNER_MESSAGE_NONE;
    InnerMessages entityValidationResult;

    int *lives = &game->lives;
    int *score = &game->score;

    if (*lives == 0) {
        *score = -*score;
        return EVALUATION_MANCHE_LOST;
    }

    if (!areHideoutsClosed(&game->map)) {
        return EVALUATION_GAME_WON;
    }


    for (int i = 0; i < MAX_CONCURRENCY; i++) {
        Component *c = &game->components[i];


        switch (c->type) {
            case COMPONENT_ENTITY: {
                Entity *entity = (Entity *) c->component;
                entityValidationResult = validate_entity(entity, &game->map, list);
                result = entityValidationResult != INNER_MESSAGE_NONE ? entityValidationResult : result;
            } break;
            case COMPONENT_ENTITIES: {
                Entities *entities = (Entities *) c->component;
                struct entities_list *el = entities->entities;
                while (el) {
                    entityValidationResult = validate_entity(el->e, &game->map, list);
                    if (entityValidationResult == INNER_MESSAGE_DESTROY_ENTITY) {
                        struct entities_list *prev = el;
                        entities->entity_num--;
                        el = el->next;
                        delete_entity_pos(prev->e->height, prev->e->width, prev->e->last, game->map);
                        destroy_entity(&entities->entities, list, prev->e);
                    } else el = el->next;
                }
            } break;
            default:
                continue;
        }
    }

    return result;
}

void evaluate_entity(InnerMessages *message, Entity *e, struct entities_list **list, Component *components, MapSkeleton map) {
    if (e->hps != 0) return;
    switch (e->type) {
        case ENTITY_TYPE__FROG: {
            *message = EVALUATION_MANCHE_LOST;
        } break;
        case ENTITY_TYPE__PROJECTILE: {
            Entities *es = (Entities*) components[(e->trueType == TRUETYPE_PROJ_FROG) ? COMPONENT_FROG_PROJECTILES_INDEX : COMPONENT_PROJECTILES_INDEX].component;
            struct entities_list *el = (struct entities_list *)es->entities;
            delete_entity_pos(e->height, e->width, e->current, map);
            destroy_entity(&el, list, e);
            es->entity_num--;
        } break;
        case ENTITY_TYPE__PLANT: {
            delete_entity_pos(e->height, e->width, e->current, map);
            e->current = getPosition(0, 0);
        } break;
    }
}

InnerMessages apply_physics(GameSkeleton *game, struct entities_list **list)
{
    InnerMessages message = INNER_MESSAGE_NONE;
    int *lives = &game->lives;
    int *score = &game->score;

    struct entities_list *el = *list;

    CollisionPacket collisionPacket;

    while (el)
    {
        Entity *entity = el->e;
        struct entities_list *innerEl = el->next;
        bool isEntityRemoved = false;

        while (innerEl)
        {
            bool isInnerEntityRemoved = false;
            Entity *innerEntity = innerEl->e;

            collisionPacket = areColliding(*entity, *innerEntity);

            switch (collisionPacket.collision_type)
            {
                case COLLISION_OVERLAPPING: {
                    bool anyEntityIsFrog = collisionPacket.e1 == TRUETYPE_FROG || collisionPacket.e2 == TRUETYPE_FROG;
                    bool anyEntityIsAngryCroc = collisionPacket.e1 == TRUETYPE_ANGRY_CROC || collisionPacket.e2 == TRUETYPE_ANGRY_CROC;
                    if (anyEntityIsFrog && anyEntityIsAngryCroc) message = EVALUATION_START_SECONDARY_CLOCK;
                } break;
                case COLLISION_DAMAGING: {
                    entity->hps--;
                    innerEntity->hps--;
                    evaluate_entity(&message, entity, list, game->components, game->map);
                    evaluate_entity(&message, innerEntity, list, game->components, game->map);
                } break;
                case COLLISION_DESTROYING: {
                    bool isEntityOneProj = collisionPacket.e1 == TRUETYPE_PROJ_FROG;
                    Entities *frog_projs = (Entities*) game->components[COMPONENT_FROG_PROJECTILES_INDEX].component;
                    if (!isEntityOneProj) {
                        innerEl = innerEl->next;
                        isInnerEntityRemoved = true;
                    } else {
                        el = el->next;
                        isEntityRemoved = true;
                    }
                    Entity *toBeDestroyed = (isEntityOneProj) ? entity : innerEntity;
                    delete_entity_pos(toBeDestroyed->height, toBeDestroyed->width, toBeDestroyed->last, game->map);
                    destroy_entity(&(frog_projs->entities), list, toBeDestroyed);
                    frog_projs->entity_num--;
                } break;
                case COLLISION_TRANSFORM: {
                    bool isEntityOneProj = collisionPacket.e1 == TRUETYPE_PROJ_FROG;
                    if (isEntityOneProj) innerEntity->trueType = TRUETYPE_CROC;
                    else entity->trueType = TRUETYPE_CROC;
                    Entities *frog_projs = (Entities*) game->components[COMPONENT_FROG_PROJECTILES_INDEX].component;
                    if (!isEntityOneProj) {
                        innerEl = innerEl->next;
                        isInnerEntityRemoved = true;
                    } else {
                        el = el->next;
                        isEntityRemoved = true;
                    }
                    Entity *toBeDestroyed = (isEntityOneProj) ? entity : innerEntity;
                    delete_entity_pos(toBeDestroyed->height, toBeDestroyed->width, toBeDestroyed->last, game->map);
                    destroy_entity(&(frog_projs->entities), list, toBeDestroyed);
                    frog_projs->entity_num--;
                } break;
            }

            if (isEntityRemoved) break;
            if (!isInnerEntityRemoved) innerEl = innerEl->next;
        }

        if (!isEntityRemoved) el = el->next;
    }

    return INNER_MESSAGE_NONE;
}

void common_timer_reset(GameSkeleton *game, int index)
{
    Clock *clock = (Clock*) game->components[index].component;
    clock->current = clock->starting;
}

void reset_main_timer(GameSkeleton *game)
{
    common_timer_reset(game, COMPONENT_CLOCK_INDEX);
}

void reset_secondary_timer(GameSkeleton *game)
{
    common_timer_reset(game, COMPONENT_TEMPORARY_CLOCK_INDEX);
}

void reset_frog(GameSkeleton *game)
{
    Entity *frog = (Entity *) game->components[COMPONENT_FROG_INDEX].component;
    delete_entity_pos(frog->height, frog->width, frog->last, game->map);
    *frog = entities_default_frog(game->map);
}

void free_entities_list(struct entities_list **list, bool full) {
    while (*list) {
        struct entities_list *next = (*list)->next;
        if (full) free((*list)->e);
        free((*list));
        *list = next;
    }
}

void free_memory(GameSkeleton *game, struct entities_list **list) {
    for (int i = 0; i < MAX_CONCURRENCY; i++) {
        Component *c = &game->components[i];
        switch(c->type) {
            case COMPONENT_CLOCK: {
                Clock *content = (Clock*)c->component;
                free(content);
            } break;
            case COMPONENT_ENTITY: {
                Entity *content = (Entity*)c->component;
                free(content);
            } break;
            case COMPONENT_ENTITIES: {
                Entities *entities = (Entities*) c->component;
                free_entities_list(&entities->entities, true);
                free(entities);
            } break;
        }
    }
    free_entities_list(list, false);
    free(game->map.hideouts);
}