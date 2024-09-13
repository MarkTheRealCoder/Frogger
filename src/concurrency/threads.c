#include "threads.h"

InnerMessages thread_polling_routine(int *buffer, GameSkeleton *game, Thread *threadList) {
    InnerMessages innerMessage = INNER_MESSAGE_NONE;

    sem_wait(&POLLING_READING);
    sem_wait(&POLLING_WRITING);
    sem_wait(&POLLING_READING);

    for (int i = 0; i < MAX_CONCURRENCY; i++) {
        int value = buffer[i];

        if (value == COMMS_EMPTY) {
            continue;
        }

        InnerMessages otherMessage = INNER_MESSAGE_NONE;

        buffer[i] = COMMS_EMPTY;
        Component *c = find_component(i, game);
        switch(c->type) {
            case COMPONENT_CLOCK:
                otherMessage = handle_clock(c, value);
                threadList[i].rules.rules[0] = value;
                break;
            case COMPONENT_ENTITY:
                otherMessage = handle_entity(c, value, true);
                break;
            case COMPONENT_ENTITIES:
                otherMessage = handle_entities(c, value);
                break;
        }

        if (otherMessage != INNER_MESSAGE_NONE) {
            innerMessage = otherMessage;
        }
    }

    sem_post(&POLLING_WRITING);

    return innerMessage;
}

void *generic_thread(void *packet) {
    Packet *p = (Packet*) packet;
    void (*producer)(void*) = p->producer;
    unsigned int id = p->id;
    unsigned int index = 0;
    while ((id >> index) != 1) index++;

    ThreadCarriage *carriage = (ThreadCarriage*) p->carriage;
    int *buffer = (int*) carriage->buffer;
    ProductionRules rules = carriage->rules;

    SystemMessage action = MESSAGE_NONE;
    while (true) {
        if (MATCH_ID(id, COMMUNICATIONS)) {
            SystemMessage msg = COMMUNICATIONS & MESSAGE_RUN;
            action = (msg != action && msg != MESSAGE_NONE) ? msg : action;
        }

        if (action == MESSAGE_RUN) {
            sem_wait(&POLLING_WRITING);
            sem_post(&POLLING_WRITING);


            producer(&rules);

            if (buffer[index] == COMMS_EMPTY)
                buffer[index] = rules.buffer;

            sem_post(&POLLING_READING);
        }
        else if (action == MESSAGE_STOP) {
            break;
        }

        sleepy(p->ms + ((rules.buffer == ACTION_PAUSE) ? 500 : 0), TIMEFRAME_MILLIS);
    }

    free(rules.rules);
    free(carriage);
    free(p);

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
                t->rules.rules[0] = (entity->type == ENTITY_TYPE__CROC) ? ACTION_WEST : ACTION_SHOOT;
                packet->ms = entity->type == ENTITY_TYPE__PLANT ? 1000 + gen_num(1000, 5000) : 125; // TODO EDIT
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
                comps[i] = getDefaultEntitiesComponent();
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

void common_timer_reset(Thread *threadList, GameSkeleton *game, int index)
{
    Clock *clock = (Clock*) game->components[index].component;
    clock->current = clock->starting;
    sem_wait(&POLLING_WRITING);
    threadList[index].rules.rules[0] = clock->current;
    sem_post(&POLLING_WRITING);
}

void reset_main_timer(Thread *threadList, GameSkeleton *game)
{
    common_timer_reset(threadList, game, COMPONENT_CLOCK_INDEX);
}

void reset_secondary_timer(Thread *threadList, GameSkeleton *game)
{
    common_timer_reset(threadList, game, COMPONENT_TEMPORARY_CLOCK_INDEX);
}

void reset_entities(Thread *threadList, GameSkeleton *game, struct entities_list **list) {
    Entity *croc, *previousCroc;
    int padding = 0;
    int y = game->map.river.y;
    getDefaultActionByY(game->map, y, true);
    bool noRemainder;
    for (int i = 1, j = 0, z = y; i <= COMPONENT_CROC_INDEXES; ++i, z = y + j * 3) {
        int currentPadding = gen_num(4, 8);
        Action action = getDefaultActionByY(game->map, z, false);
        noRemainder = (i - 1) % 2 == 0;

        croc = (Entity *) game->components[i].component;

        if (!noRemainder) previousCroc = croc;

        padding += ((action == ACTION_WEST) ? 0 : croc->width * 5) + currentPadding + (noRemainder ? 0 : previousCroc->width);

        sem_wait(&POLLING_WRITING);
        threadList[i].rules.rules[0] = action;
        sem_post(&POLLING_WRITING);

        croc->current = set_croc_position(game->map, z, padding);
        croc->trueType = choose_between(4, TRUETYPE_ANGRY_CROC, TRUETYPE_CROC, TRUETYPE_CROC, TRUETYPE_CROC);
        display_debug_string(12 + i - 1, "croc[%d] x=%d y=%d", 30, i - 1, croc->current.x, croc->current.y);
        if ((i - 1) % 2 == 1) {
            j++;
            padding = 0;
        }
        else padding = currentPadding;
    }
}

int thread_main(Screen screen, GameSkeleton *game, struct entities_list **entitiesList)
{
    erase();
    init_semaphores();
    COMMUNICATIONS = MESSAGE_RUN;

    int *buffer = CALLOC(int, MAX_CONCURRENCY);
    CRASH_IF_NULL(buffer)
    for (int i = 0; i < MAX_CONCURRENCY; i++) buffer[i] = COMMS_EMPTY;

    int *lives = &game->lives;
    int *score = &game->score;
    int threadIds = 0;
    bool running = true, drawAll = true;

    Thread *threadList = create_threads(game->components, buffer, &threadIds);
    Clock *mainClock = (Clock*) find_component(COMPONENT_CLOCK_INDEX, game)->component;

    reset_entities(threadList, game, entitiesList);

    draw(*entitiesList, &game->map, mainClock, &game->achievements, game->score, game->lives, drawAll);

    while (running)
    {
        InnerMessages result = thread_polling_routine(buffer, game, threadList);
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
                drawAll = true;
                COMMUNICATIONS = MESSAGE_RUN;
            } break;
            default:
                break;
        }

        if (!skipValidation) result = apply_validation(game, entitiesList);

        switch (result)
        {
            case POLLING_FROG_DEAD:
            case POLLING_MANCHE_LOST:
                (*lives)--;
            case EVALUATION_MANCHE_WON:
            {
                reset_main_timer(threadList, game);
                reset_secondary_timer(threadList, game);
                reset_frog(game);
                reset_entities(threadList, game, entitiesList);
                *score += 1000;
                erase();
                drawAll = true;
            } break;
            case EVALUATION_GAME_LOST:
            case EVALUATION_GAME_WON:
                running = false;
                break;
            default:
                break;
        }

        if (!running) break;

        create_new_entities(entitiesList, game->components, game->map);

        apply_physics(game, entitiesList);
        
        draw(*entitiesList, &game->map, mainClock, &game->achievements, game->score, game->lives, drawAll);
        drawAll = false;
        sleepy(50, TIMEFRAME_MILLIS);
    }

    COMMUNICATIONS = MESSAGE_STOP;
    erase();
    center_string_colored("Press any key to continue...", alloc_pair(COLOR_RED, COLOR_BLACK), 30, 20);

    for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
        pthread_join(threadList[i].id, NULL);
    }
    free(threadList);

    free(buffer);

    free_memory(game, entitiesList);

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

