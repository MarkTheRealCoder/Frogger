#include "threads.h"

InnerMessages  thread_polling_routine(int *buffer, GameSkeleton *game)
{
    InnerMessages innerMessage = INNER_MESSAGE_NONE;

    sem_wait(&POLLING_READING);
    sem_wait(&POLLING_WRITING);

    //printf("Qualcuno ha prodotto? Siotto!\n");

    for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
        int value = buffer[i];

        if (value == COMMS_EMPTY)
        {
            continue;
        }

        buffer[i] = COMMS_EMPTY;
        Component *c = find_component(i, game);
        switch(c->type)
        {
            case COMPONENT_CLOCK:
                innerMessage = handle_clock(c, value);
                break;
            case COMPONENT_ENTITY:
                innerMessage = handle_entity(c, value, true);
                break;
            case COMPONENT_ENTITIES:
                innerMessage = handle_entities(c, value);
                break;
        }
    }

    sem_post(&POLLING_WRITING);

    return innerMessage;
}

void *generic_thread(void *packet)
{
    Packet *p = (Packet*) packet;
    void (*producer)(void*) = p->producer;
    unsigned int id = p->id;
    unsigned int index = 0;
    while ((id >> index) != 1) index++;

    ThreadCarriage *carriage = (ThreadCarriage*) p->carriage;
    int *buffer = (int*) carriage->buffer;
    ProductionRules rules = carriage->rules;

    SystemMessage action = MESSAGE_NONE;
    while (true)
    {
        if (MATCH_ID(id, COMMUNICATIONS))
        {
            SystemMessage msg = COMMUNICATIONS & MESSAGE_RUN;
            action = (msg != action && msg != MESSAGE_NONE) ? msg : action;
        }

        if (action == MESSAGE_RUN)
        {
            sem_wait(&POLLING_WRITING);
            sem_post(&POLLING_WRITING);

            producer(&rules);

            if (buffer[index] == COMMS_EMPTY)
                buffer[index] = rules.buffer;

            sem_post(&POLLING_READING);
        }
        else if (action == MESSAGE_STOP)
        {
            //printf("STOP=%d\n", id);
            break;
        }

        //printf("SLEEP at INDEX=%d | TIME=%dms\n", index, p->ms);
        sleepy(p->ms, TIMEFRAME_MILLIS);
    }

    return NULL;
}

void thread_factory(int *threads, Component comp, Thread *t, int *buffer)
{
    Packet *packet = CALLOC(Packet, 1);
    CRASH_IF_NULL(packet)

    ThreadCarriage *carriage = CALLOC(ThreadCarriage, 1);
    CRASH_IF_NULL(carriage)
    carriage->buffer = buffer;
    AVAILABLE_DYNPID(packet->id, *threads);

    t->rules.rules = NULL;
    packet->ms = 0;

    switch(comp.type)
    {
        case COMPONENT_ENTITY:
        {
            Entity *entity = (Entity*) comp.component;
            if (entity->type == ENTITY_TYPE__FROG)
            {
                packet->producer = &user_listener;
            }
            else
            {
                packet->producer = &entity_move;
                t->rules.rules = CALLOC(int, 1);
                CRASH_IF_NULL(t->rules.rules)
                t->rules.rules[0] = ACTION_WEST;
                packet->ms = entity->type == ENTITY_TYPE__PLANT ? 1000 + gen_num(1000, 5000) : 1000;
            }
        } break;
        case COMPONENT_ENTITIES:
        {
            packet->producer = &entity_move;
            t->rules.rules = CALLOC(int, 1);
            CRASH_IF_NULL(t->rules.rules)
            t->rules.rules[0] = ACTION_NORTH;
            packet->ms = 200;
        } break;
        case COMPONENT_CLOCK:
        {
            packet->producer = &timer_counter;
            t->rules.rules = CALLOC(int, 2);
            CRASH_IF_NULL(t->rules.rules)
            Clock *clock = (Clock*) comp.component;
            t->rules.rules[0] = clock->current;
            t->rules.rules[1] = clock->fraction;
            packet->ms = clock->fraction;
        } break;
        default:
            break;
    }
    carriage->rules = t->rules;
    packet->carriage = carriage;
    pthread_create(&t->id, NULL, &generic_thread, packet);
}

Thread *create_threads(Component comps[MAX_CONCURRENCY], int *buffer, int *threads)
{
    Thread *threads_list = CALLOC(Thread, MAX_CONCURRENCY);
    CRASH_IF_NULL(threads_list)

    int clocks = 2;
    int projectiles = 2;

    enum ComponentType type;

    for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
        threads_list[i].component = &(comps[i]);

        type = comps[i].type;
        clocks += (type == COMPONENT_CLOCK) ? -1 : 0;
        projectiles += (type == COMPONENT_ENTITIES) ? -1 : 0;

        if (!type)
        {
            if (!clocks && projectiles)
            {
                comps[i] = (Component) {.type=COMPONENT_ENTITIES, .component=create_entities_group()};
                projectiles--;
            }

            switch (clocks)
            {
                case 1:
                case 2:
                {
                    comps[i] = getDefaultClockComponent(clocks == 2 ? CLOCK_SECONDARY : CLOCK_MAIN);
                    clocks--;
                } break;
                default:
                    break;
            }
        }

        thread_factory(threads, comps[i], &(threads_list[i]), buffer);
    }

    return threads_list;
}

int thread_main(Screen screen, GameSkeleton *game, struct entities_list **list)
{
    erase();
    init_semaphores();
    COMMUNICATIONS = MESSAGE_RUN;

    int *buffer = CALLOC(int, MAX_CONCURRENCY);
    for (int i = 0; i < MAX_CONCURRENCY; i++) buffer[i] = COMMS_EMPTY;
    int *lives = &game->lives;
    int *score = &game->score;
    int threadIds = 0;

    Thread *threadList = create_threads(game->components, buffer, &threadIds);
    Clock *mainClock = (Clock*) find_component(COMPONENT_CLOCK_INDEX, game)->component;

    make_MapSkeleton(&game->map, getPosition(MAP_START_X, MAP_START_Y), MAP_WIDTH);
    draw(*list, &game->map, mainClock, &game->achievements, game->score, game->lives, true);


    Entity* frog = ((Entity*) game->components[COMPONENT_FROG_INDEX].component);

   // display_debug_string("START | CURR_X: %d CURR_Y: %d", 40, frog->current.x, frog->current.y);
    //display_debug_string("START | LAST_X: %d LAST_Y: %d", 40, frog->last.x, frog->last.y);

    display_debug_string(10, "sidewalk.y = %d", 30, game->map.sidewalk.y);


    frog->current = getPositionWithInnerMiddleX(game->map.width, game->map.sidewalk.y, 1, 1, FROG_WIDTH);


    while (true)
    {
        //display_debug_string("LOOP | CURR_X: %d CURR_Y: %d", 40, frog->current.x, frog->current.y);
        //display_debug_string("LOOP | LAST_X: %d LAST_Y: %d", 40, frog->last.x, frog->last.y);

        InnerMessages result = thread_polling_routine(buffer, game);
        bool skipValidation = false;

        switch (result)
        {
            case POLLING_MANCHE_LOST:
            case POLLING_FROG_DEAD:
                skipValidation = true;
                break;
            case POLLING_GAME_PAUSE:
            {
                COMMUNICATIONS = MESSAGE_HALT;
                int output;
                show(screen, PS_PAUSE_MENU, &output);
                draw(*list, &game->map, mainClock, &game->achievements, game->score, game->lives, true);
                COMMUNICATIONS = MESSAGE_RUN;
            } break;
        }

        if (!skipValidation) result = apply_validation(game, list);

        switch (result)
        {
            case POLLING_FROG_DEAD:
            case POLLING_MANCHE_LOST:
                (*lives)--;
            case EVALUATION_MANCHE_WON:
            {
                // reset_main_timer(game); // todo <-
                // reset_secondary_timer(game); // todo <-
                // reset_frog(game); // todo <-
                // reset_entities(list, game); // todo <-
                *score += 1000;
            } break;
        }

        create_new_entities(list, game->components); // todo <-

        apply_physics(game, list);
        
        draw(*list, &game->map, mainClock, &game->achievements, game->score, game->lives, false);

        sleepy(50, TIMEFRAME_MILLIS);
    }

    // todo cleanup

    close_semaphores();

    return *score;
}

/**
 * Inizializza i semafori.
 */
void init_semaphores()
{
    sem_init(&COMMUNICATION_SEMAPHORE, 0, 1);
    sem_init(&POLLING_WRITING, 0, 1);
    sem_init(&POLLING_READING, 0, 1);
}

/**
 * Distrugge i semafori.
 */
void close_semaphores()
{
    sem_close(&COMMUNICATION_SEMAPHORE);
    sem_close(&POLLING_WRITING);
    sem_close(&POLLING_READING);
}

