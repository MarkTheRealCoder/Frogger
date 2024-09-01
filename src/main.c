/*#include "game/core.h"
#include "game/addons.h"
#include "game/routines.h"
#include "utils/shortcuts.h"
#include "graphics/drawing.h"
#include "utils/globals.h"*/
#include "utils/imports.h"

/*
void cleanup()
{
    endwin();
    printf("SIGSEGV occured!\n");
    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
    struct program_args parsed_program_args = addons_parse_args(argc, argv); 

    if (parsed_program_args.help)
    {
        addons_args_help(); 
        return EXIT_SUCCESS;
    }

    srand(time(NULL));
    
    Screen screen;
    init_screen(&screen);

    // Se il terminale è troppo piccolo, comunica a schermo.
    handle_screen_resize();
    signal(SIGWINCH, handle_screen_resize);
    signal(SIGSEGV, cleanup);
    
    // Impedisce di giocare a Frogger con il terminale troppo piccolo.
    if (!isScreenValid())
    {
        endwin();
        printf("Screen size is too small to play Frogger! :(\n");
        return EXIT_FAILURE;
    }

    int menuOutput = -1;

    do {
        show(screen, PS_MAIN_MENU, &menuOutput);

        switch (menuOutput) 
        {
            case MMO_OPEN_SAVING: 
                {
                    show(screen, PS_SAVINGS, &menuOutput);
                    // getGameData(output, &game);
                }
                break;
            case MMO_CREATE_SAVING: 
                {
                    show(screen, PS_CREATE_SAVING, &menuOutput);
                    menuOutput = -1;
                }
                break;
            case MMO_QUIT:
                endwin();
                return EXIT_SUCCESS;
            default:
                break;
        }
    } while (menuOutput == -1);


    struct game_threads game = { };
    setup_map(&game);
    init_game_threads(&game);


    Packet *startupPacket = NULL;

    do 
    {
        startupPacket = create_threads(&game);
        lockMancheEndedMutex();
        run_threads(&game);

        lockMancheEndedMutex();
        destroy_packet(startupPacket);
        unlockMancheEndedMutex();
    } while (isGameEnded());

    return EXIT_SUCCESS;
}
*/

typedef struct {
    void *datas;
    int buffer;
} DataBuffer;

typedef struct {
    DataBuffer data;
    int *buffer;
    sem_t *writing;
    sem_t *reading;
} Carriage;

typedef struct {
    unsigned int id;
    void *carriage;
    void (*producer)(void*);
} Packet;

static int COMMUNICATIONS = 0;

typedef enum {
    MESSAGE_RUN=3, 
    MESSAGE_HALT=2,
    MESSAGE_STOP=1, 
    MESSAGE_NONE=0
} pMessages;

#define SAFE_VALUE -999
#define MATCH_ID(id, message) ((message) >> 4) == 0 || ((message) >> 4) & (id)
#define AVAILABLE_DYNPID(result, ids)   \
{                                       \
    int copy = ids, count = 0;          \
                                        \
    while (copy & 1)                    \
    {                                   \
        count++;                        \
        copy = copy >> 1;               \
    }                                   \
                                        \
    result = 1 << (count);              \
    ids = ids | result;                 \
}

void generic_thread(void *packet) {

    Packet *p = (Packet*) packet;
        void (*producer)(void*) = p->producer;
        unsigned int id = p->id;
        unsigned int index = 0;
        while ((id >> index) != 1) index++;
    
    Carriage *carriage = p->carriage;
        int *buffer = carriage->buffer;
        DataBuffer data = carriage->data;
    

    pMessages action = MESSAGE_NONE;
    while (true) {

        if (MATCH_ID(id, COMMUNICATIONS)) {
            action  = COMMUNICATIONS & MESSAGE_RUN;
        }

        if (action == MESSAGE_RUN) {
            producer(&data);

            sem_wait(carriage->reading);
            int unlocked = 0;
            sem_getvalue(carriage->writing, &unlocked);
            if (unlocked) sem_wait(carriage->writing);

            if (buffer[index] == SAFE_VALUE) buffer[index] = data.buffer;

            sem_getvalue(carriage->writing, &unlocked);
            if (!unlocked) sem_post(carriage->writing);
            sem_post(carriage->reading);
            sleep(1);
        }
        else if (action == MESSAGE_STOP) {
            return;
        }
    }
}

void entity_move(void *_data) {
    DataBuffer *data = (DataBuffer*) _data;
    int value = (int) ((int*)data->datas)[0];
    data->buffer = value;
}

void produttore(void *_data) {
    DataBuffer *data = (DataBuffer*) _data;
    data->buffer = -1; 
    while (data->buffer == -1) {
        switch(getch()) {
            case 'A': data->buffer = LEFT;
                break;
            case 'S': data->buffer = DOWN;
                break;
            case 'W': data->buffer = UP;
                break;
            case 'D': data->buffer = RIGHT;
                break;
            case 'P': data->buffer = PAUSE;
                break;
        }
    }
}

int main(int argc, char **argv) {
    int prodLeft    = LEFT;
    int prodRight   = RIGHT;

    int *buffer = (int*) calloc(25, sizeof(int));
    buffer[0] = SAFE_VALUE;
    sem_t reading, writing;
    sem_init(&reading, 0, 1);
    sem_init(&writing, 0, 1);

    DataBuffer data = {.datas=&prodRight, .buffer=0};
    Carriage c = {.data=data, .buffer=buffer, .reading=&reading, .writing=&writing};

    Packet p = {.carriage=&c, .producer=&produttore};
    int threads = 0;
    AVAILABLE_DYNPID(p.id, threads);

    pthread_t t;
    pthread_create(&t, NULL, &generic_thread, &p);

    COMMUNICATIONS = MESSAGE_RUN;
    for (int i = 0; i < 10; i++) {
        sem_wait(&writing);
        sem_wait(&reading);
        if (buffer[0] == SAFE_VALUE && i == 0) {
            i--;
        }
        else {
            printf("Print numero %i: %i\n", i, buffer[0]);
            buffer[0] = SAFE_VALUE;
        }
        sem_post(&reading);
        sem_post(&writing);
        sleep(1);
        if (i == 3) {
            COMMUNICATIONS = MESSAGE_HALT;
        }
        if (i == 6) COMMUNICATIONS = MESSAGE_RUN;
    }
    COMMUNICATIONS = MESSAGE_STOP;
    pthread_join(t, NULL);
}



































