#include "game/core.h"
#include "game/addons.h"
#include "game/routines.h"
#include "utils/shortcuts.h"
#include "graphics/drawing.h"

void test_threads(struct game_threads *game);
void terminate();

int main(int argc, char *argv[])
{
    struct program_args parsed_program_args = addons_parse_args(argc, argv); 

    if (parsed_program_args.help)
    {
        addons_args_help(); 
        terminate(); 
    }

    srand(time(NULL));

    Screen screen;
    init_screen(&screen);
  
    /* Se il terminale e' troppo piccolo, comunica a schermo. */
    handle_screen_resize();
    signal(SIGWINCH, handle_screen_resize);

    /* Impedisce di giocare a Frogger con il terminale troppo piccolo. */
    if (!isScreenValid())
    {
        endwin();
        printf("Screen size is too small to play Frogger! :(\n");
        exit(EXIT_FAILURE);
    }

    struct game_threads game = { };
    init_game_threads(&game);

    int output;
    show(screen, PS_PAUSE_MENU, &output);

    erase();

    setup_map(&game);

    if (TEST_MODE)
    {
        test_threads(&game);
        terminate();
    }
    
    // da eseguire all'inizio di ogni partita
    create_threads(&game);

    // da eseguire alla pausa della partita
    halt_threads(&game);

    // da eseguire alla fine di ogni partita
    cancel_threads(&game);

    // da eseguire alla fine di ogni partita
    if (parsed_program_args.save_game_stats)
    {
        addons_args_save_stats(&game);
    }

    // da eseguire alla fine di ogni partita, dopo il save_game_stats
    if (parsed_program_args.quit_on_win)
    {
        cancel_threads(&game); 
        terminate();
    }

    endwin();
    return EXIT_SUCCESS;
}

void terminate()
{
    endwin();
    exit(EXIT_SUCCESS);
}

void test_threads(struct game_threads *game)
{
    Packet *beginnerPacket = create_threads(game);

    run_threads(game);

    sleepy(500, TIMEFRAME_SECONDS);
    cancel_threads(game);
    destroy_packet(beginnerPacket);
}
