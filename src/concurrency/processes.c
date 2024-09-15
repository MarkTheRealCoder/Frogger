#include "processes.h"

/**
 * Crea una pipe.
 * @param name  Il nome della pipe.
*/
pipe_t create_pipe()
{
    pipe_t p;
    HANDLE_ERROR(pipe(p.accesses) + 1);
    return p;
}

/**
 * Legge dalla pipe.
 * @param buff  Il buffer.
 * @param _pipe La pipe.
 * @param size  La dimensione del contenuto del buffer.
*/
void readFrom(void *buff, pipe_t _pipe, size_t size)
{
    HANDLE_ERROR(read(_pipe.accesses[READ], buff, size)+1);
}

/**
 * Scrive sulla pipe.
 * @param buff  Il buffer.
 * @param _pipe La pipe.
 * @param size  La dimensione del contenuto del buffer.
 */
void writeTo(void *buff, pipe_t _pipe, size_t size)
{
    HANDLE_ERROR(write(_pipe.accesses[WRITE], buff, size)+1);
}

/**
 * Legge dalla pipe se è pronta.
 * @param buff  Il buffer.
 * @param _pipe La pipe.
 * @param size  La dimensione del contenuto del buffer.
*/
bool readIfReady(void *buff, pipe_t _pipe, size_t size)
{
    fd_set set;

    /* Set di file descriptors. */
    FD_ZERO(&set);
    FD_SET(_pipe.accesses[READ], &set);

    bool result = select(FD_SETSIZE, &set, NULL, NULL, ACCEPTABLE_WAITING_TIME) > 0;

    /* Controlla se sono presenti elementi nella pipe. */
    if (result)
    {
        readFrom(buff, _pipe, size);
    }

    return result;
}

/**
 * Scrive sulla pipe se è pronta.
 * @param buff  Il buffer.
 * @param _pipe La pipe.
 * @param size  La dimensione del contenuto del buffer.
 */
bool writeIfReady(void *buff, pipe_t _pipe, size_t size)
{
    fd_set set;

    /* Set di file descriptors. */
    FD_ZERO(&set);
    FD_SET(_pipe.accesses[WRITE], &set);

    /* Controlla se non sono presenti elementi nella pipe. */
    bool result = select(FD_SETSIZE, NULL, &set, NULL, ACCEPTABLE_WAITING_TIME) > 0;

    if (result)
    {
        writeTo(buff, _pipe, size);
    }

    return result;
}

/**
 * Esegue il polling dei processi.
 * @param game          Il gioco.
 * @param processList   La lista dei processi.
 * @param isResetted    I flag di reset dei timer -
 *                      (prende l'ultimo input della pipe e lo rimuove essendo l'input prodotto con le regole non aggiornate)
 * @return              Il messaggio interno.
 */
InnerMessages process_polling_routine(GameSkeleton *game, Process *processList, bool isResetted[PIPE_SIZE])
{
    InnerMessages innerMessage = INNER_MESSAGE_NONE;

    for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
        int value = COMMS_EMPTY;

        readIfReady(&value, processList[i].comms[READ], sizeof(int));

        if (value == COMMS_EMPTY) {
            continue;
        }

        InnerMessages otherMessage = INNER_MESSAGE_NONE;
        Component *c = find_component(i, game);

        if ((i == COMPONENT_CLOCK_INDEX || i == COMPONENT_TEMPORARY_CLOCK_INDEX) && isResetted[i - COMPONENT_CLOCK_INDEX]) {
            Clock *cl = (Clock*) c->component;
            isResetted[i - COMPONENT_CLOCK_INDEX] = value != (cl->current - cl->fraction);
            continue;
        }

        switch(c->type)
        {
            case COMPONENT_CLOCK:
                otherMessage = handle_clock(c, value);
                writeTo(&value, processList[i].comms[WRITE], sizeof(int));
                break;
            case COMPONENT_ENTITY:
                otherMessage = handle_entity(c, value, true);
                break;
            case COMPONENT_ENTITIES:
                otherMessage = handle_entities(c, value);
                break;
            default:
                break;
        }

        if (otherMessage != INNER_MESSAGE_NONE) {
            innerMessage = otherMessage;
        }
    }

    return innerMessage;
}

/**
 * Invia un messaggio.
 * @param message     Il messaggio.
 * @param service_mem La memoria di servizio.
 */
void send_message(const int message, void *service_mem)
{
    memcpy(service_mem, &message, sizeof(SystemMessage));
}

/**
 * Controlla se ci sono comunicazioni.
 * @param id            L'id del processo.
 * @param service_mem   La memoria di servizio.
 * @return              Il messaggio.
 */
SystemMessage check_for_comms(const unsigned int id, void *service_mem)
{
    int message = MESSAGE_NONE;
    memcpy(&message, service_mem, sizeof(int));

    if (MATCH_ID(id, message)) {
        message = message & MESSAGE_RUN;
    }
    else {
        message = MESSAGE_NONE;
    }

    return message;
}

/**
 * Processo generico.
 * @param service_comms Il canale di comunicazione.
 * @param args          Gli argomenti.
 */
void generic_process(void *service_comms, void *args)
{
    Packet *p = (Packet*) args;
    void (*producer)(void*) = p->producer;
    unsigned int id = p->id;
    unsigned int index = 0;
    while ((id >> index) != 1) index++;

    ProcessCarriage *carriage = (ProcessCarriage *) p->carriage;
    pipe_t *comms = carriage->comms;
    ProductionRules rules;
    rules.rules = CALLOC(int, ((index == COMPONENT_CLOCK_INDEX || index == COMPONENT_TEMPORARY_CLOCK_INDEX) ? 2 : 1));
    rules.rules[0] = carriage->rules.rules[0];
    rules.rules[1] = carriage->rules.rules[1];

    CLOSE_READ(comms[READ]);
    CLOSE_WRITE(comms[WRITE]);
    SystemMessage action = MESSAGE_NONE;

    struct timeval starting, comparator;
    gettimeofday(&starting, NULL);

    while (true)
    {
        SystemMessage new_action = check_for_comms(id, service_comms);
        if (new_action != MESSAGE_NONE && new_action != action) {
            action = new_action;
        }

        if (action == MESSAGE_RUN) {
            int tmp = -1;
            if (index) {
                readIfReady(&(tmp), comms[WRITE], sizeof(int));
                if (tmp != -1) rules.rules[0] = tmp;
            }

            producer(&rules);

            gettimeofday(&comparator, NULL);
            int delay = comparator.tv_sec - starting.tv_sec;
            int udelay = comparator.tv_usec - starting.tv_usec;

            if (index || (udelay >= 100000 || delay >= 1)) {
                gettimeofday(&starting, NULL);
                writeIfReady(&(rules.buffer), comms[READ], sizeof(int));
            }
        }
        else if (action == MESSAGE_STOP) {
            munmap(service_comms, sizeof(int));
            CLOSE_READ(comms[WRITE]);
            CLOSE_WRITE(comms[READ]);
            free(rules.rules);
            break;
        }

        sleepy(p->ms + ((rules.buffer == ACTION_PAUSE) ? 500 : 0), TIMEFRAME_MILLIS);
    }
}

/**
 * Crea un nuovo processo.
 * @param processes     I processi.
 * @param service_comms Il canale di comunicazione.
 * @param _func         La funzione da eseguire.
 * @param args          Gli argomenti della funzione.
 * @return              Il processo creato.
*/
Process palloc(void *service_comms, Packet *packet)
{
    Process p = {
            .comms = ((ProcessCarriage*) packet->carriage)->comms
    };

    p.pid = fork();

    if (p.pid == 0) {
        generic_process(service_comms, (void*)packet);
        exit(EXIT_SUCCESS);
    }

    return p;
}

/**
 * Crea un nuovo processo.
 * @param processes     I processi.
 * @param service_comms Il canale di comunicazione.
 * @param p             Il processo.
 * @param c             Il componente.
 */
void process_factory(int *processes, void *service_comms, Process *p, Component c)
{
    Packet packet = {};

    AVAILABLE_DYNPID(packet.id, *processes);

    ProcessCarriage carriage = {};

    int rules[PIPE_SIZE] = {0};

    carriage.rules.rules = rules;
    carriage.comms[READ] = create_pipe();
    carriage.comms[WRITE] = create_pipe();
    packet.ms = 0;

    switch(c.type)
    {
        case COMPONENT_ENTITY:
        {
            Entity *entity = (Entity*) c.component;
            if (entity->type == ENTITY_TYPE__FROG) {
                packet.producer = &user_listener;
            }
            else {
                packet.producer = &entity_move;
                rules[0] = (entity->type == ENTITY_TYPE__CROC) ? ACTION_WEST : ACTION_SHOOT;
                packet.ms = entity->type == ENTITY_TYPE__PLANT ? 3000 + gen_num(2000, 5000) : 400;
                if (entity->type == ENTITY_TYPE__PLANT) {
                    add_timer(packet.id);
                }
            }
        } break;
        case COMPONENT_ENTITIES:
        {
            packet.producer = &entity_move;
            rules[0] = (packet.id & (1 << (COMPONENT_FROG_PROJECTILES_INDEX))) ? ACTION_NORTH : ACTION_SOUTH;
            packet.ms = 200;
        } break;
        case COMPONENT_CLOCK:
        {
            packet.producer = &timer_counter;
            Clock *clock = (Clock*) c.component;
            rules[0] = clock->current;
            rules[1] = clock->fraction;
            packet.ms = clock->fraction;
        } break;
        default:
            break;
    }

    packet.carriage = &carriage;
    *p = palloc(service_comms, &packet);
    p->comms = CALLOC(pipe_t, 2);
    p->comms[READ].accesses[0] = carriage.comms[READ].accesses[0];
    p->comms[READ].accesses[1] = carriage.comms[READ].accesses[1];
    p->comms[WRITE].accesses[0] = carriage.comms[WRITE].accesses[0];
    p->comms[WRITE].accesses[1] = carriage.comms[WRITE].accesses[1];
    CLOSE_READ(p->comms[WRITE]);
    CLOSE_WRITE(p->comms[READ]);
}

/**
 * Crea i processi.
 * @param game          Il gioco.
 * @param service_comms Il canale di comunicazione.
 * @param processes     I processi.
 * @return              La lista dei processi.
 */
Process *create_processes(GameSkeleton *game, void* service_comms, int *processes)
{
    Process *processList = CALLOC(Process, MAX_CONCURRENCY);
    CRASH_IF_NULL(processList)

    Component *comps = game->components;

    int clocks = 2;
    int projectiles = 2;

    enum ComponentType type;

    for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
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

        process_factory(processes, service_comms, &processList[i], comps[i]);
    }

    return processList;
}

/**
 * Resetta il gioco.
 * @param processList   La lista dei processi.
 * @param game          Il gioco.
 * @param list          La lista delle entità.
 */
void reset_game_processes(Process *processList, GameSkeleton *game, struct entities_list **list)
{
    int *tmp_buffer = reset_game(game, list);

    for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
        if (tmp_buffer[i] != COMMS_EMPTY) {
            writeIfReady(&(tmp_buffer[i]), processList[i].comms[WRITE], sizeof(int));
        }
    }

    free(tmp_buffer);
}

/**
 * Resetta il timer secondario.
 * @param processList   La lista dei processi.
 * @param game          Il gioco.
 */
void reset_temporary_clock_process(Process *processList, GameSkeleton *game)
{
    int value;
    reset_secondary_timer(&value, game);
    writeIfReady(&value, processList[COMPONENT_TEMPORARY_CLOCK_INDEX].comms[WRITE], sizeof(int));
}

/**
 * Processo principale.
 * @param screen        Lo schermo.
 * @param game          Il gioco.
 * @param entitiesList  La lista delle entità.
 * @return              Il punteggio finale.
 */
int process_main(Screen screen, GameSkeleton *game, struct entities_list **entitiesList)
{
    void* service_comms = mmap(NULL, sizeof(int),
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    erase();

    int *lives = &game->lives;
    int *score = &game->score;
    bool running = true, drawAll = true;
    int processes = 0;

    Process *processList = create_processes(game, service_comms, &processes);
    Clock *mainClock = (Clock*) find_component(COMPONENT_CLOCK_INDEX, game)->component;
    Clock *secClock = (Clock*) find_component(COMPONENT_TEMPORARY_CLOCK_INDEX, game)->component;

    draw(*entitiesList, &game->map, mainClock, secClock, game->score, game->lives, drawAll);

    reset_game_processes(processList, game, entitiesList);
    send_message(create_message(MESSAGE_RUN, processes - (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX))), service_comms);

    bool isResetted[PIPE_SIZE] = { false };

    while (running)
    {
        InnerMessages result = process_polling_routine(game, processList, isResetted);
        bool skipValidation = false;

        switch (result)
        {
            case POLLING_MANCHE_LOST:
            case POLLING_FROG_DEAD:
                skipValidation = true;
                break;
            case POLLING_GAME_PAUSE:
            {
                send_message(MESSAGE_HALT, service_comms);

                int output;
                show(screen, PS_PAUSE_MENU, &output);

                if (output) {
                    running = false;
                    *score = 0;
                }
                else {
                    drawAll = true;
                    send_message(create_message(MESSAGE_RUN, processes - (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX))), service_comms);
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
                send_message(create_message(MESSAGE_RUN, (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX))), service_comms);
            } break;
            case EVALUATION_STOP_SECONDARY_CLOCK:
            {
                send_message(create_message(MESSAGE_HALT, (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX))), service_comms);
                reset_temporary_clock_process(processList, game);
                isResetted[1] = true;
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
                reset_game_processes(processList, game, entitiesList);
                *score += result == EVALUATION_MANCHE_WON ? 1000 : 0;
                clear_screen();
                drawAll = true;
                for (int i = 0; i < PIPE_SIZE; i++)
                {
                    isResetted[i] = true;
                }
            } break;
            case EVALUATION_GAME_LOST:
                *score = -*score;
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

    send_message(create_message(MESSAGE_STOP, processes), service_comms);

    clear_screen();

    center_string_colored("The frog is going to sleep...", alloc_pair(COLOR_RED, COLOR_BLACK), 30, 20);
    center_string_colored("Wait a few seconds, then press a button!", alloc_pair(COLOR_RED, COLOR_BLACK), 41, 21);

    for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
        CLOSE_READ(processList[i].comms[READ]);
        CLOSE_WRITE(processList[i].comms[WRITE]);
        waitpid(processList[i].pid, NULL, 0);
    }
    free(processList);

    free_memory(game, entitiesList);
    wait(NULL);

    munmap(service_comms, sizeof(int));
    return *score;
}
