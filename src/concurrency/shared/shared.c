#include "shared.h"

PollingResult handle_clock(Component *component, int value)
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

PollingResult handle_entity(Component *component, int value, int canPause)
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

PollingResult handle_entities(Component *component, int value)
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

void entity_move(void *_rules) {
    ProductionRules *rules = (ProductionRules*)_rules;
    int value = rules->rules[0];
    rules->buffer = value;
}

void timer_counter(void *_rules) {
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
                //validate_entity(entity, game->map);
            } break;
            case COMPONENT_ENTITIES:
            {
                Entities *entities = (Entities *) c->component;
                //validate_entities(entities, game->map);
            } break;
            default:
                continue;
        }
    }

    // validate_entities(*list, game->map);
    // close hideouts

    return INNER_MESSAGE_NONE;
}

InnerMessages apply_physics(GameSkeleton *game, struct entities_list **list)
{
    int *lives = &game->lives;
    int *score = &game->score;



    return INNER_MESSAGE_NONE;
}