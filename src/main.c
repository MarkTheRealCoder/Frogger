#include "commons/imports.h"

enum MAIN_TASK propose_version_menu(Screen screen, int *menuOutput);
enum MAIN_TASK propose_main_menu(Screen screen, int *menuOutput);
void checkForTerminate(enum MAIN_TASK task, WINDOW *window);
int slaveMain(Screen screen, WINDOW *window, enum MAIN_TASK *currentTask);

int main(int argc, char *argv[])
{
    struct program_args parsed_program_args = addons_parse_args(argc, argv);
    enum MAIN_TASK currentTask = KEEP_GOING;

    if (parsed_program_args.help)
    {
        addons_args_help();
        return EXIT_SUCCESS;
    }

    srand(time(NULL));

    Screen screen;
    WINDOW *window = init_screen(&screen);

    // Se il terminale è troppo piccolo, comunica a schermo.
    //handle_screen_resize();
    //signal(SIGWINCH, handle_screen_resize);

    int exitCode = EXIT_FAILURE;

    do {
        exitCode = slaveMain(screen, window, &currentTask);
    } while (currentTask != TERMINATE);

    return exitCode;
}

int slaveMain(Screen screen, WINDOW *window, enum MAIN_TASK *currentTask)
{
    int threadsOrProcessesMenu = 0;

    *currentTask = propose_version_menu(screen, &threadsOrProcessesMenu);
    checkForTerminate(*currentTask, window);

    int mainMenu = -1;

    GameSkeleton game = {
            .lives = TOTAL_LIVES,
            .score = 0,
            .achievements = {
                    .nodes = 0,
                    .last = NULL
            }
    };

    *currentTask = propose_main_menu(screen, &mainMenu);
    checkForTerminate(*currentTask, window);

    make_MapSkeleton(&game.map, getPosition(MAP_START_X, MAP_START_Y), MAP_WIDTH);

    int finalScore;
    struct entities_list *entities = create_default_entities(&game);
    if (!threadsOrProcessesMenu) finalScore = thread_main(screen, &game, &entities);
    else finalScore = process_main(screen, &game, &entities);

    show(screen, (finalScore > 0 ? PS_WIN : PS_LOST), &finalScore);

    return EXIT_SUCCESS;
}

enum MAIN_TASK propose_version_menu(Screen screen, int *menuOutput)
{
    enum MAIN_TASK resultTask = KEEP_GOING;

    do {
        show(screen, PS_VERSION_MENU, menuOutput);

        switch (*menuOutput)
        {
            case VMO_THREADS:
                *menuOutput = 0;
                break;
            case VMO_PROCESSES:
                *menuOutput = 1;
                break;
            case VMO_QUIT:
                resultTask = TERMINATE;
            default:
                break;
        }
    } while (*menuOutput == -1);

    return resultTask;
}

enum MAIN_TASK propose_main_menu(Screen screen, int *menuOutput)
{
    enum MAIN_TASK resultTask = KEEP_GOING;

    do {
        show(screen, PS_MAIN_MENU, menuOutput);

        switch (*menuOutput)
        {
            case MMO_START_NEW:
                break;
            case MMO_QUIT:
                resultTask = TERMINATE;
            default:
                *menuOutput = 0;
                break;
        }
    } while (*menuOutput == -1);

    return resultTask;
}

void checkForTerminate(enum MAIN_TASK task, WINDOW *window)
{
    if (task == TERMINATE)
    {
        reset_color_pairs();
        endwin();
        delwin(window);
        exit(EXIT_SUCCESS);
    }
}
