#include "commons/imports.h"

void handle_sigenv()
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
    signal(SIGSEGV, handle_sigenv); // todo remove after
    
    // Impedisce di giocare a Frogger con il terminale troppo piccolo.
    if (!isScreenValid())
    {
        Position closeNotice = {
                getCenteredX(_FROGGER_SCREEN_CLOSE_NOTICE_LENGTH),
                getCenteredY(0) + 2
        };
        display_string(closeNotice, COLOR_RED, _FROGGER_SCREEN_CLOSE_NOTICE, _FROGGER_SCREEN_CLOSE_NOTICE_LENGTH);
        sleepy(5, TIMEFRAME_SECONDS);

        endwin();
        return EXIT_FAILURE;
    }

    int threadsOrProcessesMenu = -1, mainMenu = -1;

    /*
     * Version menu.
     */
    do {
        show(screen, PS_VERSION_MENU, &threadsOrProcessesMenu);

        switch (threadsOrProcessesMenu)
        {
            case VMO_THREADS:
                break;
            case VMO_PROCESSES:
                break;
            case VMO_QUIT:
                endwin();
                return EXIT_SUCCESS;
            default:
                break;
        }
    } while (threadsOrProcessesMenu == -1);

    /*
     * Main menu.
     */

    GameSkeleton game = {
            .current_plants = 0,
            .current_projectiles = 0,
            .current_frog_projectiles = 0,
            .achievements = {
                    .nodes = 0,
                    .last = NULL
            }
    };

    bool loadedFromFile = false;

    do {
        show(screen, PS_MAIN_MENU, &mainMenu);

        switch (mainMenu)
        {
            case MMO_OPEN_SAVING: 
                {
                    show(screen, PS_SAVINGS, &mainMenu);
                    // getGameData(menuOutput, &game);
                    loadedFromFile = true;
                }
                break;
            case MMO_CREATE_SAVING: 
                {
                    show(screen, PS_CREATE_SAVING, &mainMenu);
                    mainMenu = -1;
                }
                break;
            case MMO_QUIT:
                endwin();
                return EXIT_SUCCESS;
            default:
                break;
        }
    } while (mainMenu == -1);

    struct entities_list *entities = create_default_entities(&game, loadedFromFile);
    //thread_main(&game, &entities);

    return EXIT_SUCCESS;
}

/*
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
    */
/*

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


*/

/**
 * \^/
   -*-
   /|\
   \^/
 */
