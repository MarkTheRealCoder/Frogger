#include "threads.h"

PollingResult  thread_polling_routine(int *buffer, GameSkeleton *game)
{
    PollingResult pollingResult = POLLING_NONE;

    sem_wait(&POLLING_READING);
    sem_wait(&POLLING_WRITING);

    printf("Qualcuno ha prodotto? Siotto!\n");

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
                pollingResult = handle_clock(c, value);
                break;
            case COMPONENT_ENTITY:
                pollingResult = handle_entity(c, value, true);
                break;
            case COMPONENT_ENTITIES:
                pollingResult = handle_entities(c, value);
                break;
        }
    }

    sem_post(&POLLING_WRITING);

    return pollingResult;
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
    while (true) {

        if (MATCH_ID(id, COMMUNICATIONS)) {
            SystemMessage msg = COMMUNICATIONS & MESSAGE_RUN;
            action = (msg != action && msg != MESSAGE_NONE) ? msg : action;
        }

        if (action == MESSAGE_RUN) {

            sem_wait(&POLLING_WRITING);
            sem_post(&POLLING_WRITING);

            producer(&rules);
            //printf("BUFFER for %d | ADDR=%x\n", index, buffer);
            if (buffer[index] == COMMS_EMPTY) buffer[index] = rules.buffer;

            sem_post(&POLLING_READING);
        }
        else if (action == MESSAGE_STOP) {
            printf("STOP=%d\n", id);
            break;
        }

        printf("SLEEP at INDEX=%d | TIME=%dms\n", index, p->ms);
        sleepy(p->ms, TIMEFRAME_MILLIS);
    }
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
            packet->ms = 500;
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

Thread *create_threads(Component comps[MAX_CONCURRENCY], int *buffer, int *threads) {
    Thread *threads_list = CALLOC(Thread, MAX_CONCURRENCY);
    CRASH_IF_NULL(threads_list)
    int clocks = 2;
    int projectiles = 2;

    enum ComponentType type;

    for (int i = 0; i < MAX_CONCURRENCY; i++) {
        threads_list[i].component = &(comps[i]);

        type = comps[i].type;
        clocks += (type == COMPONENT_CLOCK) ? -1 : 0;
        projectiles += (type == COMPONENT_ENTITIES) ? -1 : 0;

        if (!type) {
            if (!clocks && projectiles) {
                comps[i] = (Component) {.type=COMPONENT_ENTITIES, .component=create_entities_group()};
                projectiles--;
            }
            switch (clocks) {
                case 1: {
                    comps[i] = (Component) {.type=COMPONENT_CLOCK, .component=create_clock(2, CLOCK_SECONDARY)};
                    clocks--;
                } break;
                case 2: {
                    comps[i] = (Component) {.type=COMPONENT_CLOCK, .component=create_clock(120, CLOCK_MAIN)};
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

int thread_main(Screen scr, GameSkeleton *game, struct entities_list **list) {
    erase();
    init_semaphores();
    int *buffer = CALLOC(int, MAX_CONCURRENCY);
    int threads = 0, score = 0, lives = TOTAL_LIVES;

    Thread *threadList = create_threads(game->components, buffer, &threads);
    COMMUNICATIONS = MESSAGE_RUN;
    //draw(*list, &game->map, , &game->achievements, score, lives, true); todo redo.
    while (true) {
        PollingResult result = thread_polling_routine(buffer, game);
        switch (result) {
            case POLLING_MANCHE_LOST:
            case POLLING_FROG_DEAD: {

            } break;
            case POLLING_GAME_PAUSE: {
                COMMUNICATIONS = MESSAGE_HALT;
                int output;
                show(scr, PS_PAUSE_MENU, &output);
            } break;
        }

        // Validazione delle entità
            //validate_entities(*list, game->map);

        // Creazione di nuove entità
            //create_new_entities(list, game->components, game->map)

        // todo: Maybe together?
            // Collisioni
            // Rimozione entità e aggiornamento

        // Display
            // draw();

        sleepy(50, TIMEFRAME_MILLIS);
    }

    close_semaphores();
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