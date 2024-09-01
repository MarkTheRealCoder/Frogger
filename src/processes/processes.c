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
        array[i] = create_pipe(va_arg(nl, char*));
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
void readfrm(void *buff, pipe_t _pipe, size_t size) 
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
void writeto(void *buff, pipe_t _pipe, size_t size)
{
    HANDLE_ERROR(write(_pipe.accesses[WRITE], buff, size)+1);
}

/**
 * Legge dalla pipe se è pronta.
 * @param buff  Il buffer.
 * @param _pipe La pipe.
 * @param size  La dimensione del contenuto del buffer.
*/
bool readifready(void *buff, pipe_t _pipe, size_t size) 
{
    fd_set set;

    /* Set di file descriptors. */
    FD_ZERO(&set);
    FD_SET(_pipe.accesses[READ], &set);

    bool result = select(FD_SETSIZE, &set, NULL, NULL, ACCEPTABLE_WAITING_TIME) > 0;

    /* Controlla se sono presenti elementi nella pipe. */
    if (result)
    {
        readfrm(buff, _pipe, size);
    }

    return result;
}

/**
 * Scrive sulla pipe se è pronta.
 * @param buff  Il buffer.
 * @param _pipe La pipe.
 * @param size  La dimensione del contenuto del buffer.
 */
bool writeifready(void *buff, pipe_t _pipe, size_t size) 
{
    fd_set set;

    /* Set di file descriptors. */
    FD_ZERO(&set);
    FD_SET(_pipe.accesses[WRITE], &set);

    /* Controlla se non sono presenti elementi nella pipe. */
    bool result = select(FD_SETSIZE, NULL, &set, NULL, ACCEPTABLE_WAITING_TIME) > 0;

    if (result)
    {
        writeto(buff, _pipe, size);
    }

    return result;
}

int create_message(pMessages action, int receivers) {
    return (int) action + (receivers << 4);
}

int send_message(int message, void *service_mem) {
    memcpy(service_mem, &message, sizeof(int));
}

pMessages check_for_comms(int id, void * service_mem) {
    int message = MESSAGE_NONE;
    memcpy(&message, service_mem, sizeof(int));
    if (MATCH_ID(id, message)) {
        message = message & MESSAGE_RUN;
    }
    return message;
}

void generic_process(int id, int service_comms, void (*producer)(void*), void *args) {
    void *service_mem = mmap(0, SERVICE_SIZE, PROT_READ, MAP_SHARED, service_comms, 0);
    pMessages action = MESSAGE_NONE;
    while (true) {
        pMessages new_action = check_for_comms(id, service_mem);
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
    else if (p.pid == -1) 
    {
        p.status = STATUS_IDLE;
    }
    
    return p;
}



int process_main(int argc, char **argv) {
    int processes = 0;
    int service_comms = shm_open(SERVICE_NAME, O_CREAT | O_RDWR, 0666);
    HANDLE_ERROR(service_comms);
    HANDLE_ERROR(lseek(service_comms, SERVICE_SIZE - 1, SEEK_SET));
    void *service_mem = mmap(0, SERVICE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, service_comms, 0);
    if (service_mem == MAP_FAILED) {
        perror("mmap failed!\n");
        close(service_comms);
        exit(EXIT_FAILURE);
    }

    //Screen scr;
    //init_screen(&scr);


    /*MAIN MENU*/
    while (true) {

    }

    /*GAME*/
    while (true) {
        
    }
        /*PAUSE MENU*/

    //endwin();
    munmap(service_mem, SERVICE_SIZE);
    close(service_comms);
}

void draw(struct entity_node *es, MapSkeleton *map, TimerPacket timer, StringList *achievements, int score, bool drawAll) {
    /*Draw Map if not already drawn*/
    const static Position map_position = { getCenteredX(MAP_WIDTH), 5 };
    if (drawAll) *map = display_map(map_position, MAP_WIDTH, NULL);

    /*Draw timer*/
    display_clock((Position){ getCenteredX(0) + 25, 2 };, timer.current_time, timer.max_time);

    /*Draw achievements*/
    const static Position achievementTitlePosition = { getCenteredX(12) + 72, getCenteredY(25) - 2 };
    const static Position achievementPosition = { getCenteredX(30) + 75, getCenteredY(25) };
    if (drawAll) display_string(achievementTitlePosition, COLOR_RED, "Achievements", 12);
    display_achievements(achievementPosition, 34, 25, *achievements);

    /*Draw SCORE and HPS*/
    const static Position hpsPosition = { getCenteredX(FROG_HPS) - 45, 3 };
    const static Position scorePosition = { getCenteredX(12), 3 };

    display_hps(hpsPosition, 3, 2);
    display_string(scorePosition, COLOR_RED, "Score: XXXXX", 12);

    for (int i = 0; i < 5; i++) {
        struct entity_node *ec = es;
        while (ec != NULL) {
            if (getPriorityByEntityType(ec->entity.type) == i) {
                /*Bisogna modificare la struct entity: bisogna inserire più informazioni, come il tipo dell'entità (DISPLAY) e la vecchia posizione*/
                display_entity(COLORCODES_CROC_A, getArt(&ec->entity), (Position){ec->entity.x, ec->entity.y}, (Position){ec->entity.x, ec->entity.y}, *map);
            }
            ec = ec->next;
        }
    }
}
