#include "processes.h"


/**
 * Crea una pipe.
 * @param name  Il nome della pipe.
*/
pipe_t create_pipe(char name[PIPE_NAME]) 
{
    pipe_t p;

    p.name = CALLOC(char, PIPE_NAME);
    strcpy(p.name, name);

    HANDLE_ERROR(pipe(p.accesses) + 1);

    int len = strlen(name);

    p.name = REALLOC(char, p.name, len + TERM);
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

    va_list nl;
    va_start(nl, size);
    
    for (int i = 0; i < size; i++) 
    {
        array[i] = create_pipe(va_arg(nl, char*)); // CLION: Leak of memory allocated in function 'create_pipe'
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

SystemMessage create_message(SystemMessage action, int receivers)
{
    return action + (receivers << 4);
}

void send_message(int message, void *service_mem)
{
    memcpy(service_mem, &message, sizeof(SystemMessage));
}

SystemMessage check_for_comms(int id, void *service_mem)
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
        if (new_action != MESSAGE_NONE && new_action != action) action = new_action;
        if (action == MESSAGE_RUN) {
            producer(args);
        } else if (action == MESSAGE_STOP) {
            munmap(service_mem, SERVICE_SIZE);
            close(service_comms);
            break;
        }
    }
}


/**
 * Crea un nuovo processo.
 * @param pname Il nome del processo.
 * @param _func Il puntatore alla funzione.
 * @param pkg   Il pacchetto.
*/
Process palloc(int *processes, int service_comms, void (*_func)(void*), void *args)
{
    Process p = {.status=0};

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



int process_main(int argc, char **argv) {
    // ADD A GAME STRUCT FOR FAST LOADING DATA
    int processes = 0;
    Screen scr;
    init_screen(&scr);

    /*MAIN MENU*/
    do {
        int output = -1;
        show(scr, PS_MAIN_MENU, &output);
        switch (output) {
            case MMO_OPEN_SAVING: {
                show(scr, PS_SAVINGS, &output);
                // getGameData(output, &game);
            }
                break;
            case MMO_CREATE_SAVING: {
                show(scr, PS_CREATE_SAVING, &output);
                output = -1;
            }
                break;
            case MMO_QUIT: return 0;
        }
    } while (output == -1);

    int service_comms = shm_open(SERVICE_NAME, O_CREAT | O_RDWR, 0666);
    HANDLE_ERROR(service_comms);
    HANDLE_ERROR(lseek(service_comms, SERVICE_SIZE - 1, SEEK_SET));
    void *service_mem = mmap(0, SERVICE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, service_comms, 0);
    if (service_mem == MAP_FAILED) {
        perror("mmap failed!\n");
        close(service_comms);
        exit(EXIT_FAILURE);
    }

    Position map_position = { getCenteredX(MAP_WIDTH), 5 };
    MapSkeleton map = display_map(map_position, MAP_WIDTH, NULL);

    /*GAME*/
    while (true) {
        
    }
        /*PAUSE MENU*/

    //endwin();
    munmap(service_mem, SERVICE_SIZE);
    close(service_comms);
}
