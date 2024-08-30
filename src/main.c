#include "game/core.h"
#include "game/addons.h"
#include "game/routines.h"
#include "utils/shortcuts.h"
#include "graphics/drawing.h"

void test_threads(struct game_threads *game);

int main(int argc, char *argv[])
{
    int output;
    Screen scr;
    init_screen(&scr);
    //show(scr, PS_PAUSE_MENU, &output);
    //MapSkeleton map = display_map((Position){.x=10,.y=5}, 100, NULL);
    
    //wgetch(stdscr);

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
    endwin();

    
    return EXIT_SUCCESS;
}

void test_threads(struct game_threads *game)
{
    Packet *beginnerPacket = create_threads(game);

    struct entity_node *node = game->entity_node;
    while (node != NULL) {
        if (node->entity.type == ENTITY_TYPE__FROG) break;
        node = node->next;
    }

    run_threads(game);

    sleepy(500, TIMEFRAME_SECONDS);
    cancel_threads(game);
    destroy_packet(beginnerPacket);
}
