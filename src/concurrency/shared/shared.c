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
            .component = &list->e,
            .type = COMPONENT_ENTITY
        };

        handle_entity(&entityComponent, value, false);
        list = list->next;

        /*
        if (isActionMovement(value)) update_position(&el->e, value);
        else if (value == ACTION_SHOOT) el->e.readyToShoot = true;
        el = el->next;
         */
    }

    return INNER_MESSAGE_NONE;
}

Component *find_component(const int index, GameSkeleton *game)
{
    return &game->components[index];
}

Component **find_components(GameSkeleton *game, ...)
{
    Component **foundComponents = NULL;

    va_list args;
    va_start(args, game);

    int index;
    int counter = 0;

    while ((index = va_arg(args, int)) != -1)
    {
        if (index < 0 || index >= MAX_CONCURRENCY)
        {
            break;
        }

        if (counter == 0)
        {
            foundComponents = MALLOC(Component *, 1);
        }
        else
        {
            foundComponents = REALLOC(Component *, foundComponents, counter + 1);
        }
        CRASH_IF_NULL(foundComponents)

        foundComponents[counter] = find_component(index, game);
        counter++;
    }

    va_end(args);

    return foundComponents;
}

void update_position(Entity *e, const Action action)
{
    if (!isActionMovement(action))
    {
        return;
    }

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

    e->last = e->current;
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

    while (list)
    {
        Entity *comparison = list->e;
        int x = comparison->current.x, y = comparison->current.y;
        int width = comparison->width, height = comparison->height;

        if (e != comparison)
        {
            bool collisionInRangeX = x <= lastEntityPos.x && lastEntityPos.x <= x + width;
            bool collisionInRangeY = y <= lastEntityPos.y && lastEntityPos.y <= y + height;

            if (collisionInRangeX && collisionInRangeY)
            {
                return getPosition(0, 0);
            }
        }
        list = list->next;
    }

    return lastEntityPos;
}

void remove_entity_from_list(struct entities_list **list, Entity *e) {
    struct entities_list *pivot = *list;
    bool isFirst = true;
    while (pivot && pivot->e != e) {
        if (pivot->next->e == e) {
            isFirst = false;
            break;
        }
        pivot = pivot->next;
    }
    if (isFirst) {
        *list = pivot->next;
    } else {
        struct entities_list *next = pivot->next;
        pivot->next = pivot->next->next;
        free(next);
    }
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
            bool notWithinBoundaries = !WITHIN_BOUNDARIES(x, y, (*map)) || !WITHIN_BOUNDARIES(x + 2, y, (*map));
            int entityInsideOfHideout = isEntityInsideOfHideout(entity, map);

            if (notWithinBoundaries || !entityInsideOfHideout)
            {
                *x = *lastX;
                *y = *lastY;
            }
            else
            {
                entity->last = invalidate_position(entity, *list);
            }

            if (entityInsideOfHideout)
            {
                map->hideouts[entityInsideOfHideout - 1].x = 0;
                map->hideouts[entityInsideOfHideout - 1].y = 0;
            }
        } break;
        case ENTITY_TYPE__CROC:
        {
            if (!WITHIN_BOUNDARIES(x, y, (*map)))
            {
                bool isActionWest = getDefaultActionByY(*map, *y, false) == ACTION_WEST;
                bool invalid = (isActionWest && *x + entity->width < map->sidewalk.x) || !isActionWest;

                if (invalid)
                {
                    entity->current = reset_croc_position(*map, *y);
                }
            }
        } break;
        case ENTITY_TYPE__PLANT:
        {
            if (!WITHIN_BOUNDARIES(x, y, (*map)))
            {
                entity->readyToShoot = false;
            }
        } break;
        case ENTITY_TYPE__PROJECTILE:
        {
            if (!WITHIN_BOUNDARIES(x, y, (*map)))
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

InnerMessages apply_validation(GameSkeleton *game, struct entities_list **list)
{
    int *lives = &game->lives;
    int *score = &game->score;

    if (*lives == 0)
    {
        *score = -*score;
        return EVALUATION_MANCHE_LOST;
    }

    if (areHideoutsClosed(&game->map))
    {
        return EVALUATION_MANCHE_WON;
    }

    for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
        Component *c = &game->components[i];

        switch (c->type)
        {
            case COMPONENT_ENTITY:
            {
                Entity *entity = (Entity *) c->component;
                validate_entity(entity, &game->map, list);
            } break;
            case COMPONENT_ENTITIES:
            {
                Entities *entities = (Entities *) c->component;
                struct entities_list *el = entities->entities;
                while (el) {
                    InnerMessages result = validate_entity(el->e, &game->map, list);
                    if (result == INNER_MESSAGE_DESTROY_ENTITY) {
                        struct entities_list *prev = el;
                        el = el->next;
                        destroy_entity(&entities->entities, list, prev->e);
                    }
                    else el = el->next;
                }
            } break;
            default:
                continue;
        }
    }

    return INNER_MESSAGE_NONE;
}

InnerMessages apply_physics(GameSkeleton *game, struct entities_list **list)
{
    int *lives = &game->lives;
    int *score = &game->score;

    struct entities_list *el = *list;

    CollisionPacket collisionPacket;

    while (el)
    {
        Entity *entity = el->e;
        struct entities_list *innerEl = el->next;

        while (innerEl)
        {
            Entity *innerEntity = innerEl->e;

            collisionPacket = areColliding(*entity, *innerEntity);

            switch (collisionPacket.collision_type)
            {
                case COLLISION_OVERLAPPING:
                    break;
                case COLLISION_DAMAGING:
                    break;
                case COLLISION_AVOIDED:
                    break;
            }

            innerEl = innerEl->next;
        }

        el = el->next;
    }

    return INNER_MESSAGE_NONE;
}