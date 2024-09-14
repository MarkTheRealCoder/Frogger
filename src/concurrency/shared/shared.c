#include "shared.h"


static Timer *GLOBAL_TIMERS = NULL;



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
    e->moved = true;
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
    //if (e->trueType == TRUETYPE_FROG) display_debug_string(24, "X: %i, Y: %i", 80, lastEntityPos.x, lastEntityPos.y);
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
                /*
                if (e->trueType == TRUETYPE_FROG || comparison->trueType == TRUETYPE_FROG) {
                    display_debug_string(12 + (pos++ % 10), "%i INVALIDATED FOR %i: %i", 80, pos, e->trueType == TRUETYPE_FROG, lastEntityPos.y);
                }*/
                return getPosition(0, 0);
            }
        }
        list = list->next;
    }

    return lastEntityPos;
}

void remove_entity_from_list(struct entities_list **list, Entity *e)
{
    struct entities_list *pivot = *list;
    struct entities_list *prev = NULL;

    while (pivot) {
        if (pivot->e == e)
            break;
        prev = pivot;
        pivot = pivot->next;
    }

    if (!pivot)
        return;

    if (prev)
        prev->next = pivot->next;
    else
        *list = pivot->next;

    free(pivot);
}

void invalidate_entity(Entity *e) {
    e->valid = false;
}

void destroy_entity(struct entities_list **el, struct entities_list **list, Entity *e) {
    if (*el) remove_entity_from_list(el, e);
    if (*list) remove_entity_from_list(list, e);
    free(e);
}

void handle_invalid_entities(struct entities_list **list, Component components[MAX_CONCURRENCY]) {
    struct entities_list *el = *list;
    while (el) {
        if (el->e->valid) el = el->next;
        else {
            struct entities_list *current = el;
            el = el->next;
            Component *c = &components[current->e->trueType == TRUETYPE_PROJ_FROG ? COMPONENT_FROG_PROJECTILES_INDEX : COMPONENT_PROJECTILES_INDEX];
            Entities *es = (Entities *) c->component;
            destroy_entity(&es->entities, list, current->e);
            es->entity_num--;
        }
    }
}

InnerMessages validate_entity(Entity *entity, const MapSkeleton *map, struct entities_list **list, int index)
{
    int *x = &(entity->current.x);
    int *y = &(entity->current.y);

    int *lastX = &(entity->last.x);
    int *lastY = &(entity->last.y);

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

InnerMessages apply_validation(GameSkeleton *game, struct entities_list **list)
{
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
                entityValidationResult = validate_entity(entity, &game->map, list, i);
                result = entityValidationResult != INNER_MESSAGE_NONE ? entityValidationResult : result;
            } break;
            case COMPONENT_ENTITIES: {
                Entities *entities = (Entities *) c->component;
                struct entities_list *el = entities->entities;
                while (el) {
                    entityValidationResult = validate_entity(el->e, &game->map, list, i);
                    if (entityValidationResult == INNER_MESSAGE_DESTROY_ENTITY) {
                        struct entities_list *prev = el;
                        entities->entity_num--;
                        el = el->next;
                        delete_entity_pos(prev->e->height, prev->e->width, prev->e->last, game->map);
                        invalidate_entity(prev->e);
                    } else el = el->next;
                }
            } break;
            default:
                continue;
        }
    }

    return result;
}

bool evaluate_entity(InnerMessages *message, Entity *e, struct entities_list **list, Component *components, MapSkeleton map)
{
    if (e->hps > 0)
        return false;

    switch (e->type) {
        case ENTITY_TYPE__FROG: {
            *message = EVALUATION_MANCHE_LOST;
        } break;
        case ENTITY_TYPE__PROJECTILE: {
            Entities *es = (Entities*) components[(e->trueType == TRUETYPE_PROJ_FROG) ? COMPONENT_FROG_PROJECTILES_INDEX : COMPONENT_PROJECTILES_INDEX].component;
            struct entities_list *el = (struct entities_list *) es->entities;
            delete_entity_pos(e->height, e->width, e->current, map);
            delete_entity_pos(e->height, e->width, e->last, map);
            invalidate_entity(e);
            es->entity_num--;
        } break;
        case ENTITY_TYPE__PLANT: {
            delete_entity_pos(e->height, e->width, e->current, map);
            delete_entity_pos(e->height, e->width, e->last, map);
            e->current = getPosition(0, 0);
            e->hps = 2;
        } break;
    }
    return true;
}

InnerMessages apply_physics(GameSkeleton *game, struct entities_list **list)
{
    InnerMessages message = INNER_MESSAGE_NONE;
    static InnerMessages backup = INNER_MESSAGE_NONE;

    int *lives = &game->lives;
    int *score = &game->score;

    struct entities_list *el = *list;

    CollisionPacket collisionPacket;
    EntityType eligibleForReset = ENTITY_TYPE__EMPTY;

    while (el) {
        Entity *entity = el->e;
        struct entities_list *innerEl = el->next;

        while (innerEl) {
            Entity *innerEntity = innerEl->e;

            collisionPacket = areColliding(*entity, *innerEntity);

            switch (collisionPacket.collision_type) {
                case COLLISION_AVOIDED: {
                    if (collisionPacket.e1 == TRUETYPE_FROG || collisionPacket.e2 == TRUETYPE_FROG) {
                        Entity *aFrog = (collisionPacket.e1 == TRUETYPE_FROG) ? entity : innerEntity;
                        if (eligibleForReset == ENTITY_TYPE__EMPTY && !(game->map.river.y <= aFrog->current.y && aFrog->current.y < game->map.sidewalk.y)) {
                            eligibleForReset = ENTITY_TYPE__CROC;
                        }
                    }
                } break;
                case COLLISION_OVERLAPPING: {
                    bool anyEntityIsFrog = collisionPacket.e1 == TRUETYPE_FROG || collisionPacket.e2 == TRUETYPE_FROG;
                    bool anyEntityIsAngryCroc = collisionPacket.e1 == TRUETYPE_ANGRY_CROC || collisionPacket.e2 == TRUETYPE_ANGRY_CROC;
                    if (anyEntityIsFrog && anyEntityIsAngryCroc && message == INNER_MESSAGE_NONE) message = EVALUATION_START_SECONDARY_CLOCK;
                    if (anyEntityIsFrog) {
                        Entity *notAFrog = (collisionPacket.e1 == TRUETYPE_FROG) ? innerEntity : entity;
                        if (notAFrog->moved) {
                            delete_entity_pos(FROG_HEIGHT, FROG_WIDTH, ((Entity *)game->components[COMPONENT_FROG_INDEX].component)->last, game->map);
                            handle_entity(&game->components[COMPONENT_FROG_INDEX], getDefaultActionByY(game->map, entity->current.y, false), false);
                        }
                        eligibleForReset = ENTITY_TYPE__CROC;
                    }
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
                    Entity *toBeDestroyed = (isEntityOneProj) ? entity : innerEntity;
                    delete_entity_pos(toBeDestroyed->height, toBeDestroyed->width, toBeDestroyed->last, game->map);
                    invalidate_entity(toBeDestroyed);
                    frog_projs->entity_num--;
                } break;
                case COLLISION_TRANSFORM: {
                    bool isEntityOneProj = collisionPacket.e1 == TRUETYPE_PROJ_FROG;
                    if (isEntityOneProj) innerEntity->trueType = TRUETYPE_CROC;
                    else entity->trueType = TRUETYPE_CROC;
                    Entities *frog_projs = (Entities*) game->components[COMPONENT_FROG_PROJECTILES_INDEX].component;
                    Entity *toBeDestroyed = (isEntityOneProj) ? entity : innerEntity;
                    delete_entity_pos(toBeDestroyed->height, toBeDestroyed->width, toBeDestroyed->last, game->map);
                    invalidate_entity(toBeDestroyed);
                    frog_projs->entity_num--;
                } break;
            }

            innerEl = innerEl->next;
        }

        el = el->next;
    }

    if (message == EVALUATION_START_SECONDARY_CLOCK) backup = message;
    else if (message == INNER_MESSAGE_NONE && backup == EVALUATION_START_SECONDARY_CLOCK) {
        message = EVALUATION_STOP_SECONDARY_CLOCK;
        backup = INNER_MESSAGE_NONE;
    }
    if (eligibleForReset == ENTITY_TYPE__EMPTY && message == INNER_MESSAGE_NONE) message = EVALUATION_MANCHE_LOST;

    return message;
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

void common_timer_reset(int *buffer, GameSkeleton *game, int index)
{
    Clock *clock = (Clock*) game->components[index].component;
    clock->current = clock->starting;
    *buffer = clock->current;
}

void reset_main_timer(int *buffer, GameSkeleton *game)
{
    common_timer_reset(buffer, game, COMPONENT_CLOCK_INDEX);
}

void reset_secondary_timer(int *buffer, GameSkeleton *game)
{
    common_timer_reset(buffer, game, COMPONENT_TEMPORARY_CLOCK_INDEX);
}

void reset_frog(GameSkeleton *game)
{
    Entity *frog = (Entity *) game->components[COMPONENT_FROG_INDEX].component;
    delete_entity_pos(frog->height, frog->width, frog->last, game->map);
    *frog = entities_default_frog(game->map);
}

int reset_crocodile(int *buffer, int index, GameSkeleton *game, int prevPadding) {
    int line = (index == 0) ? 0 : (int)(index / 2);
    int y = game->map.river.y + (line * 3);
    Action action = getDefaultActionByY(game->map, y, (!index));
    Entity *croc = (Entity *) game->components[index + 1].component;

    int currentPadding = gen_num(4, 8);
    int padding = ((action == ACTION_WEST) ? 0 : croc->width * 5) + currentPadding + (index % 2 ? prevPadding : 0);

    *buffer = action;

    croc->current = set_croc_position(game->map, y, padding);
    croc->trueType = choose_between(4, TRUETYPE_ANGRY_CROC, TRUETYPE_CROC, TRUETYPE_CROC, TRUETYPE_CROC);

    return currentPadding + croc->width;
}

void reset_crocodiles(int *buffer, GameSkeleton *game) {
    for (int i = 0, prevPadding = 0; i < COMPONENT_CROC_INDEXES; i++) {
        prevPadding = reset_crocodile(&buffer[i+1], i, game, prevPadding);
    }
}

void reset_plants(GameSkeleton *game) {
    for (int i = COMPONENT_CROC_INDEXES + 1; i < COMPONENT_CLOCK_INDEX; i++) {
        Component *c = &game->components[i];
        Entity *plant = (Entity *) c->component;
        plant->current = getPosition(0, 0);
        plant->readyToShoot = false;
    }
}

void destroy_all_projectiles(GameSkeleton *game, struct entities_list **list) {
    for (int i = COMPONENT_FROG_PROJECTILES_INDEX; i <= COMPONENT_PROJECTILES_INDEX; i++) {
        Component *c = &game->components[i];
        Entities *projs = (Entities *) c->component;
        projs->entity_num = 0;

        struct entities_list *el = projs->entities;

        while (el) {
            struct entities_list *current = el;
            el = el->next;
            destroy_entity(&projs->entities, list, current->e);
        }
    }
}

int *reset_game(GameSkeleton *game, struct entities_list **list) {
    int *buffer = CALLOC(int, MAX_CONCURRENCY);
    for (int i = 0; i < MAX_CONCURRENCY; i++) buffer[i] = COMMS_EMPTY;
    reset_main_timer(&buffer[COMPONENT_CLOCK_INDEX], game);
    reset_secondary_timer(&buffer[COMPONENT_TEMPORARY_CLOCK_INDEX], game);
    reset_frog(game);
    reset_crocodiles(buffer, game);
    reset_plants(game);
    destroy_all_projectiles(game, list);
    return buffer;
}

void reset_moved(struct entities_list *list)
{
    while (list)
    {
        list->e->moved = false;
        list = list->next;
    }
}

void clear_timers() {
    Timer *pivot = GLOBAL_TIMERS;
    while (pivot) {
        Timer *current = pivot;
        pivot = pivot->next;
        free(current);
    }
}

int destroy_timer(unsigned int index) {
    Timer *pivot = GLOBAL_TIMERS;
    Timer *prev = NULL;
    while (pivot && pivot->id != index) {
        prev = pivot;
        pivot = pivot->next;
    }

    if (!pivot) return false;

    if (!prev) GLOBAL_TIMERS = pivot->next;
    else prev->next = pivot->next;
    free(pivot);
    return true;
}

Timer *scroll_timers(int *index) {
    if (index && *index == 0) return NULL;
    Timer *pivot = GLOBAL_TIMERS;
    while (pivot) {
        if (index == NULL && pivot->next == NULL) break;
        else if (index) {
            if (*index < 0 && pivot->id == -*index) {
                pivot = NULL;
                break;
            } else if (pivot->id == *index) break;
        }
        pivot = pivot->next;
    }
    return pivot;
}

void update_timer(unsigned int index) {
    Timer *pivot = scroll_timers(&index);
    if (!pivot) return;
    gettimeofday(&pivot->start, NULL);
}

int add_timer(unsigned int index) {
    int tmp = -(int)index;
    Timer *pivot = scroll_timers(&tmp);
    if (pivot) return -1;
    pivot = scroll_timers(NULL);
    Timer *new = CALLOC(Timer, 1);
    new->id = index;
    new->next = NULL;
    if (pivot) pivot->next = new;
    else GLOBAL_TIMERS = new;
    update_timer(index);
}

int time_elapsed(unsigned int index) {
    Timer *pivot = scroll_timers(&index);

    if (!pivot) return -1;

    struct timeval tmp;
    gettimeofday(&tmp, NULL);
    return tmp.tv_sec - pivot->start.tv_sec;
}

void gen_plants(GameSkeleton *game) {
    for (int i = COMPONENT_CROC_INDEXES + 1; i < COMPONENT_CLOCK_INDEX; i++) {
        int zeroIndex = (i % (COMPONENT_CROC_INDEXES + 1));
        Component *c = &game->components[i];
        Entity *plant = (Entity *) c->component;
        unsigned int id = (1 << i);
        if (!WITHIN_BOUNDARIES(plant->current.x, plant->current.y, game->map) && time_elapsed(id) >= 6 + 3 * zeroIndex) {
            update_timer(id);
            int section = (int)(game->map.width / 3);
            int x = (section * zeroIndex) + gen_num(5, section - 5) + game->map.garden.x;
            plant->current = getPosition(x, game->map.garden.y + FROG_HEIGHT);
        }
    }
}