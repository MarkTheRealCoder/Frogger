#include "processes.h"

/**
 * Crea una pipe.
 * @param name  Il nome della pipe.
*/
pipe_t create_pipe(char name[PIPE_NAME]) 
{
    pipe_t p;

    p.name = CALLOC(char, PIPE_NAME);
    CRASH_IF_NULL(p.name)
    strcpy(p.name, name);

    HANDLE_ERROR(pipe(p.accesses) + 1);

    int len = strlen(name);

    p.name = REALLOC(char, p.name, len + TERM);
    CRASH_IF_NULL(p.name)
    p.name[len] = '\0';

    return p;
}

/**
 * Crea multiple pipe.
 * @param size  La dimensione dell'array.
 * @param ...   I nomi delle pipe.
*/
pipe_t *create_pipes(int size, ...) 
{
    pipe_t *array = CALLOC(pipe_t, size);
    CRASH_IF_NULL(array)

    va_list nl;
    va_start(nl, size);
    
    for (int i = 0; i < size; i++) 
    {
        array[i] = create_pipe(va_arg(nl, char*)); // TODO CLION: Leak of memory allocated in function 'create_pipe'
    }

    va_end(nl);

    return array;
}

/**
 * Legge dalla pipe.
 * @param buff  Il buffer.
 * @param _pipe La pipe.
 * @param size  La dimensione del contenuto del buffer.
*/
void readFrom(void *buff, pipe_t _pipe, size_t size)
{
    //printf("----- %s\n", _pipe.name);
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
SystemMessage check_for_comms(const int id, void *service_mem)
{
    SystemMessage message = MESSAGE_NONE;
    memcpy(&message, service_mem, sizeof(SystemMessage));

    if (MATCH_ID(id, message))
    {
        message = message & MESSAGE_RUN;
    }

    return message;
}

void generic_process(int id, int service_comms, void (*producer)(void*), void *args)
{
    void *service_mem = mmap(0, SERVICE_SIZE, PROT_READ, MAP_SHARED, service_comms, 0);
    SystemMessage action = MESSAGE_NONE;

    while (true)
    {
        SystemMessage new_action = check_for_comms(id, service_mem);
        if (new_action != MESSAGE_NONE && new_action != action)
        {
            action = new_action;
        }

        if (action == MESSAGE_RUN)
        {
            producer(args);
        }
        else if (action == MESSAGE_STOP)
        {
            munmap(service_mem, SERVICE_SIZE);
            close(service_comms);
            break;
        }
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
Process palloc(int *processes, int service_comms, void (*_func)(void*), void *args)
{
    Process p = { .status=0 };

    AVAILABLE_DYNPID(p.dynamic_pid, *processes);

    p.pid = fork();
    
    if (p.pid == 0)
    {
        generic_process(p.dynamic_pid, service_comms, _func, args);
        exit(EXIT_SUCCESS);
    }

    if (p.pid == -1)
    {
        p.status = STATUS_IDLE;
    }

    return p;
}

int process_main(Screen screen, GameSkeleton *game, struct entities_list **entitiesList) {

    int service_comms = shm_open(SERVICE_NAME, O_CREAT | O_RDWR, 0666);
    HANDLE_ERROR(service_comms);
    HANDLE_ERROR(lseek(service_comms, SERVICE_SIZE - 1, SEEK_SET));
    void *service_mem = mmap(0, SERVICE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, service_comms, 0);
    if (service_mem == MAP_FAILED) {
        perror("mmap failed!\n");
        close(service_comms);
        exit(EXIT_FAILURE);
    }

    erase();


    int *lives = &game->lives;
    int *score = &game->score;
    bool running = true, drawAll = true;
    int processes = 0;

    Process *processList = create_processes(game->components, &processes);
    Clock *mainClock = (Clock*) find_component(COMPONENT_CLOCK_INDEX, game)->component;
    Clock *secClock = (Clock*) find_component(COMPONENT_TEMPORARY_CLOCK_INDEX, game)->component;

    draw(*entitiesList, &game->map, mainClock, secClock, &game->achievements, game->score, game->lives, drawAll);

    //COMMUNICATIONS = create_message(MESSAGE_RUN, threadIds - (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX))); todo
    reset_game_processes(buffer, processList, game, entitiesList);

    while (running)
    {
        InnerMessages result = process_polling_routine(buffer, game, processList);
        bool skipValidation = false;

        switch (result)
        {
            case POLLING_MANCHE_LOST:
            case POLLING_FROG_DEAD:
                skipValidation = true;
                break;
            case POLLING_GAME_PAUSE:
            {
                //COMMUNICATIONS = MESSAGE_HALT; todo
                int output;
                show(screen, PS_PAUSE_MENU, &output);
                if (output)
                {
                    running = false;
                    *score = 0;
                }
                else {
                    drawAll = true;
                    //COMMUNICATIONS = create_message(MESSAGE_RUN, threadIds - (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX))); todo
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
            case EVALUATION_START_SECONDARY_CLOCK: {
                //COMMUNICATIONS = create_message(MESSAGE_RUN, (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX))); todo
            } break;
            case EVALUATION_STOP_SECONDARY_CLOCK: {
                //COMMUNICATIONS = create_message(MESSAGE_HALT, (1 << (COMPONENT_TEMPORARY_CLOCK_INDEX))); todo
                reset_temporary_clock(processList, game);
            } break;
            case POLLING_FROG_DEAD:
            case POLLING_MANCHE_LOST:
            case EVALUATION_MANCHE_LOST:
                (*lives)--;
                if (!*lives)
                {
                    running = false;
                    *score = -*score;
                    break;
                }
            case EVALUATION_MANCHE_WON:
            {
                reset_game_processes(buffer, processList, game, entitiesList);
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

        if (!running) break;

        draw(*entitiesList, &game->map, mainClock, secClock, &game->achievements, game->score, game->lives, drawAll);
        reset_moved(*entitiesList);
        drawAll = false;
        sleepy(50, TIMEFRAME_MILLIS);
    }

    //COMMUNICATIONS = MESSAGE_STOP; todo

    clear_screen();

    center_string_colored("The frog is going to sleep...", alloc_pair(COLOR_RED, COLOR_BLACK), 30, 20);
    center_string_colored("Wait a few seconds!", alloc_pair(COLOR_RED, COLOR_BLACK), 30, 21);

    /*for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
        pthread_join(processList[i].id, NULL); todo
    }*/
    free(processList);

    free_memory(game, entitiesList);


    munmap(service_mem, SERVICE_SIZE);
    close(service_comms);
    return *score;

}
