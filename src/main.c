#include "game/core.h"
#include "game/addons.h"
#include "game/routines.h"
#include "utils/shortcuts.h"
#include "graphics/drawing.h"
#include "utils/globals.h"

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

    /* Se il terminale è troppo piccolo, comunica a schermo. */
    handle_screen_resize();
    signal(SIGWINCH, handle_screen_resize);
    signal(SIGSEGV, cleanup);
    
    /* Impedisce di giocare a Frogger con il terminale troppo piccolo. */
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
