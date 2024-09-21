#include "threads.h"

static int GLOBAL_INDEX = 0;
static int INNER_INDEX = 0;

/**
 * Routine di polling dei thread.
 * @param buffer        Il buffer di comunicazione.
 * @param game          La struttura del gioco.
 * @param threadList    La lista dei thread.
 * @return              Il messaggio interno.
 */
InnerMessages thread_polling_routine(Event *buffer, GameSkeleton *game, Thread *threadList)
{
    InnerMessages innerMessage = INNER_MESSAGE_NONE;

    sem_wait(&POLLING_READING);
    sem_wait(&POLLING_WRITING);

    for (int i = 0; INNER_INDEX != GLOBAL_INDEX; i++)
    {
        Event value = buffer[(INNER_INDEX + i) % DOUBLE_MAX_CONCURRENCY];

        //buffer[i % DOUBLE_MAX_CONCURRENCY];

        Component *c = find_component(value.index, game);

        InnerMessages otherMessage = INNER_MESSAGE_NONE;

        switch(c->type)
        {
            case COMPONENT_CLOCK:
                otherMessage = handle_clock(c, value.action);
                break;
            case COMPONENT_ENTITY:
                otherMessage = handle_entity(c, value.action, true);
                break;
            case COMPONENT_ENTITIES:
                otherMessage = handle_entities(c, value.action);
                break;
            default:
                break;
        }

        if (otherMessage != INNER_MESSAGE_NONE) {
            innerMessage = otherMessage;
        }

        if ((INNER_INDEX + i) % DOUBLE_MAX_CONCURRENCY == GLOBAL_INDEX) break;
    }

    INNER_INDEX = GLOBAL_INDEX;
    sem_post(&POLLING_WRITING);
    sem_post(&POLLING_READING);

    return innerMessage;
}

/**
 * Funzione generica per i thread.
 * @param packet Il pacchetto.
 */
void *generic_thread(void *packet)
{
    Packet *p = (Packet*) packet;
    void (*producer)(void*) = p->producer;
    unsigned int id = p->id;
    unsigned int index = 0;
    while ((id >> index) != 1) index++;

    ThreadCarriage *carriage = (ThreadCarriage*) p->carriage;
    Event *buffer = (Event*) carriage->buffer;
    ProductionRules rules = carriage->rules;

    SystemMessage action = MESSAGE_NONE;

    while (true)
    {
        pthread_mutex_lock(&MUTEX);
        pthread_mutex_unlock(&MUTEX);

        if (MATCH_ID(id, COMMUNICATIONS)) {
            SystemMessage msg = COMMUNICATIONS & MESSAGE_RUN;
            action = (msg != action && msg != MESSAGE_NONE) ? msg : action;
        }

        if (action == MESSAGE_RUN) {
            sem_wait(&POLLING_READING);
            sem_post(&POLLING_READING);
            sem_wait(&POLLING_WRITING);

            producer(&rules);

            buffer[GLOBAL_INDEX] = (Event){.index=index, .action=rules.buffer};
            GLOBAL_INDEX = (GLOBAL_INDEX + 1) % DOUBLE_MAX_CONCURRENCY;

            sem_post(&POLLING_WRITING);
        }
        else if (action == MESSAGE_STOP) {
            break;
        }

        sleepy(p->ms + ((rules.buffer == ACTION_PAUSE) ? 500 : 0), TIMEFRAME_MILLIS);
    }

    free(carriage);
    free(p);

    return NULL;
}

/**
 * Factory per i thread.
 * @param threads   Il numero di thread.
 * @param comp      Il componente.
 * @param t         Il thread.
 * @param buffer    Il buffer.
 */
void thread_factory(int *threads, Component comp, Thread *t, Event *buffer)
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
            if (entity->type == ENTITY_TYPE__FROG) {
                packet->producer = &user_listener;
            }
            else {
                packet->producer = &entity_move;
                t->rules.rules = CALLOC(int, 1);
                CRASH_IF_NULL(t->rules.rules)
                t->rules.rules[0] = (entity->type == ENTITY_TYPE__CROC) ? ACTION_WEST : ACTION_SHOOT;
                packet->ms = entity->type == ENTITY_TYPE__PLANT ? 3000 + gen_num(2000, 5000) : 400;
                if (entity->type == ENTITY_TYPE__PLANT) {
                    add_timer(packet->id);
                }
            }
        } break;
        case COMPONENT_ENTITIES:
        {
            packet->producer = &entity_move;
            t->rules.rules = CALLOC(int, 1);
            CRASH_IF_NULL(t->rules.rules)
            t->rules.rules[0] = (packet->id & (1 << (COMPONENT_FROG_PROJECTILES_INDEX))) ? ACTION_NORTH : ACTION_SOUTH;
            packet->ms = 200;
        } break;
        case COMPONENT_CLOCK:
        {
            packet->producer = &timer_counter;
            t->rules.rules = CALLOC(int, 1);
            CRASH_IF_NULL(t->rules.rules)
            Clock *clock = (Clock*) comp.component;
            t->rules.rules[0] = clock->fraction;
            packet->ms = clock->fraction;
        } break;
        default:
            break;
    }
    carriage->rules = t->rules;
    packet->carriage = carriage;
    pthread_create(&t->id, NULL, &generic_thread, packet);
}

/**
 * Crea i thread.
 * @param comps     I componenti.
 * @param buffer    Il buffer.
 * @param threads   Il numero di thread.
 * @return          La lista dei thread.
 */
Thread *create_threads(Component comps[MAX_CONCURRENCY], Event *buffer, int *threads)
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

        if (!type) {
            if (!clocks && projectiles) {
                comps[i] = getDefaultEntitiesComponent();
                projectiles--;
            }

            switch (clocks)
            {
                case 1:
                case 2:
                {
                    comps[i] = getDefaultClockComponent(clocks == 1 ? CLOCK_SECONDARY : CLOCK_MAIN);
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

/**
 * Resetta il gioco.
 * @param buffer        Il buffer.
 * @param threadList    La lista dei thread.
 * @param game          La struttura del gioco.
 * @param list          La lista delle entita'.
 */
void reset_game_threads(Event *buffer, Thread *threadList, GameSkeleton *game, struct entities_list **list)
{
    int *tmp_buffer = reset_game(game, list);

    sem_wait(&POLLING_WRITING);
    for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
        if (tmp_buffer[i] != COMMS_EMPTY) {
            threadList[i].rules.rules[0] = tmp_buffer[i];
        }
    }
    GLOBAL_INDEX = 0;
    INNER_INDEX = 0;
    sem_post(&POLLING_WRITING);

    free(tmp_buffer);
}

/**
 * Funzione principale dei thread.
 * @param screen        La struttura dello schermo.
 * @param game          La struttura del gioco.
 * @param entitiesList  La lista delle entita'.
 * @return              Il punteggio.
 */
int thread_main(Screen screen, GameSkeleton *game, struct entities_list **entitiesList)
{
    erase();
    init_semaphores();

    Event *buffer = CALLOC(Event, MAX_CONCURRENCY * 2);
    CRASH_IF_NULL(buffer)

    int *lives = &game->lives;
    int *score = &game->score;
    int threadIds = 0;
    bool running = true, drawAll = true;

    pthread_mutex_lock(&MUTEX);

    Thread *threadList = create_threads(game->components, buffer, &threadIds);
    Clock *mainClock = (Clock*) find_component(COMPONENT_CLOCK_INDEX, game)->component;
    Clock *secClock = (Clock*) find_component(COMPONENT_TEMPORARY_CLOCK_INDEX, game)->component;

    draw(*entitiesList, &game->map, mainClock, secClock, game->score, game->lives, drawAll);

    COMMUNICATIONS = create_message(MESSAGE_RUN, threadIds - (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX)));
    reset_game_threads(buffer, threadList, game, entitiesList);
    pthread_mutex_unlock(&MUTEX);

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
                pthread_mutex_lock(&MUTEX);
                COMMUNICATIONS = MESSAGE_HALT;
                pthread_mutex_unlock(&MUTEX);

                int output;
                show(screen, PS_PAUSE_MENU, &output);

                if (output) {
                    running = false;
                    *score = 0;
                }
                else {
                    drawAll = true;
                    pthread_mutex_lock(&MUTEX);
                    COMMUNICATIONS = create_message(MESSAGE_RUN, threadIds - (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX)));
                    pthread_mutex_unlock(&MUTEX);
                }
            } break;
            default:
                break;
        }

        if (!skipValidation) {
            gen_plants(game);
            result = apply_validation(game, entitiesList);

            create_new_entities(entitiesList, game->components, game->map);

            if (result == INNER_MESSAGE_NONE) {
                result = apply_physics(game, entitiesList);
            }
            handle_invalid_entities(entitiesList, game->components);
        }

        switch (result)
        {
            case EVALUATION_START_SECONDARY_CLOCK:
            {
                pthread_mutex_lock(&MUTEX);
                COMMUNICATIONS = create_message(MESSAGE_RUN, (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX)));
                pthread_mutex_unlock(&MUTEX);
            } break;
            case EVALUATION_STOP_SECONDARY_CLOCK:
            {
                pthread_mutex_lock(&MUTEX);
                COMMUNICATIONS = create_message(MESSAGE_HALT, (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX)));
                reset_secondary_timer(game);
                pthread_mutex_unlock(&MUTEX);
            } break;
            case POLLING_FROG_DEAD:
            case POLLING_MANCHE_LOST:
            case EVALUATION_MANCHE_LOST:
                (*lives)--;
                if (!*lives) {
                    running = false;
                    *score = -*score;
                    break;
                }
            case EVALUATION_MANCHE_WON:
            {
                reset_game_threads(buffer, threadList, game, entitiesList);
                *score += result == EVALUATION_MANCHE_WON ? 1000 : 0;
                clear_screen();
                drawAll = true;
            } break;
            case EVALUATION_GAME_LOST:
            case EVALUATION_GAME_WON:
                running = false;
                break;
            default:
                break;
        }

        if (!running) {
            break;
        }

        draw(*entitiesList, &game->map, mainClock, secClock, game->score, game->lives, drawAll);
        reset_moved(*entitiesList);
        drawAll = false;
        sleepy(50, TIMEFRAME_MILLIS);
    }

    COMMUNICATIONS = MESSAGE_STOP;

    clear_screen();

    center_string_colored("The frog is going to sleep...", alloc_pair(COLOR_RED, COLOR_BLACK), 30, 20);
    center_string_colored("Wait a few seconds, then press a button!", alloc_pair(COLOR_RED, COLOR_BLACK), 41, 21);

    for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
        pthread_join(threadList[i].id, NULL);
    }

    free(threadList);
    free(buffer);
    clear_timers();
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
    pthread_mutex_init(&MUTEX, NULL);
}

/**
 * Distrugge i semafori.
 */
void close_semaphores()
{
    sem_close(&COMMUNICATION_SEMAPHORE);
    sem_close(&POLLING_WRITING);
    sem_close(&POLLING_READING);
    pthread_mutex_destroy(&MUTEX);
}
