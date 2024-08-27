#include "game/core.h"
#include "game/addons.h"
#include "utils/shortcuts.h"
#include "graphics/drawing.h"

void test_threads(struct game_threads *game);

int main(int argc, char *argv[])
{
    int output;
    Screen scr;
    init_screen(&scr);
    //show(scr, PS_PAUSE_MENU, &output);
    StringNode *last = NULL;
    addStringToList(&last, alloc_pair(COLOR_RED, COLOR_BLACK), "Ciao bello come va?");
    addStringToList(&last, alloc_pair(COLOR_GREEN, COLOR_BLACK), "Hey hey sei uno scozzese!");
    addStringToList(&last, alloc_pair(COLOR_RED, COLOR_BLACK), "Cazzo culo cervello di gallina$$$");
    addStringToList(&last, alloc_pair(COLOR_BLUE, COLOR_BLACK), "4 MILIARDI HHHHHHHHHHHHHHHHHHHHHHHHH");
    addStringToList(&last, alloc_pair(COLOR_RED, COLOR_BLACK), "Ricordi che bello quell'iguana skibidi toilet?");
    StringList list = {.nodes=5, .last=last};
    display_achievements((Position){.x=10, .y=5}, 15, 10, list);
    wgetch(stdscr);
    endwin();

    /*
    srand(time(NULL));
    struct program_args parsed_program_args = addons_parse_args(argc, argv); 

    if (parsed_program_args.help)
    {
        addons_args_help(); 
        return EXIT_SUCCESS;
    }

    struct game_threads game = { };
    init_game_threads(&game); 

    if (TEST_MODE)
    {
        test_threads(&game);
        return EXIT_SUCCESS;
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
        return EXIT_SUCCESS;
    }
    */
    return EXIT_SUCCESS;
}

void test_threads(struct game_threads *game)
{
    Packet *beginnerPacket = create_threads(game);

    run_threads(game);
    
    sleepy(500, TIMEFRAME_MILLIS);
    halt_threads(game);
    
    sleepy(500, TIMEFRAME_MILLIS);
    run_threads(game);

    sleepy(500, TIMEFRAME_MILLIS);
    cancel_threads(game);
    destroy_packet(beginnerPacket);
}
