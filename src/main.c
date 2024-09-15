#include "commons/imports.h"

enum MAIN_TASK propose_version_menu(Screen screen, int *menuOutput);
enum MAIN_TASK propose_main_menu(Screen screen, int *menuOutput);
void handle_termination(const enum MAIN_TASK task, WINDOW *terminal);
int game_main(Screen screen, WINDOW *window, enum MAIN_TASK *currentTask);

int main(int argc, char *argv[])
{
    struct program_args parsed_program_args = addons_parse_args(argc, argv);
    enum MAIN_TASK currentTask = KEEP_GOING;

    if (parsed_program_args.help) {
        addons_args_help();
        return EXIT_SUCCESS;
    }

    srand(time(NULL));

    Screen screen;
    WINDOW *terminal = init_screen(&screen);

    // Se il terminale è troppo piccolo, comunica a schermo.
    handle_screen_resize();
    // Rimane in ascolto di un segnale di ridimensionamento del terminale.
    signal(SIGWINCH, handle_screen_resize);

    int exitCode = EXIT_FAILURE;

    do {
        exitCode = game_main(screen, terminal, &currentTask);
    } while (currentTask != TERMINATE);

    return exitCode;
}

/**
 * Funzione principale del gioco.
 * @param screen La struttura dello schermo.
 * @param terminal La finestra del terminale.
 * @param currentTask Il task corrente.
 * @return Il codice di uscita.
 */
int game_main(const Screen screen, WINDOW *terminal, enum MAIN_TASK *currentTask)
{
    int versionChoice = 0;
    *currentTask = propose_version_menu(screen, &versionChoice);
    handle_termination(*currentTask, terminal);

    int mainMenu = -1;
    *currentTask = propose_main_menu(screen, &mainMenu);
    handle_termination(*currentTask, terminal);

    GameSkeleton game = {
            .lives = TOTAL_LIVES,
            .score = 0,
            .achievements = {
                    .nodes = 0,
                    .last = NULL
            }
    };

    make_MapSkeleton(&game.map, getPosition(MAP_START_X, MAP_START_Y), MAP_WIDTH);
    struct entities_list *entities = create_default_entities(&game);

    int finalScore = 0;

    if (versionChoice) {
        finalScore = process_main(screen, &game, &entities);
    }
    else {
        finalScore = thread_main(screen, &game, &entities);
    }

    show(screen, (finalScore > 0 ? PS_WIN : PS_LOST), &finalScore);

    return EXIT_SUCCESS;
}

/**
 * Propone il menu di scelta tra thread e processi.
 * @param screen La struttura dello schermo.
 * @param menuOutput L'output del menu.
 * @return Il task corrente.
 */
enum MAIN_TASK propose_version_menu(const Screen screen, int *menuOutput)
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

/**
 * Propone il menu principale.
 * @param screen La struttura dello schermo.
 * @param menuOutput L'output del menu.
 * @return Il task corrente.
 */
enum MAIN_TASK propose_main_menu(const Screen screen, int *menuOutput)
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

/**
 * Esegue la terminazione del gioco.
 * @param task Il task corrente.
 * @param terminal La finestra del terminale.
 */
void handle_termination(const enum MAIN_TASK task, WINDOW *terminal)
{
    if (task == TERMINATE) {
        reset_color_pairs();
        endwin();
        delwin(terminal);
        exit(EXIT_SUCCESS);
    }
}
