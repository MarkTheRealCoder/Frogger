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

    return POLLING_NONE;
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

    return POLLING_NONE;
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

    return POLLING_NONE;
}


