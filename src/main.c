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
    
    handle_screen_resize();
    signal(SIGWINCH, handle_screen_resize);

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

    int map_width = 100;
    Position map_position = { getCenteredX(map_width), 5 };

    MapSkeleton map = display_map(map_position, map_width, NULL);
    
    for (int i = 0, counter = 0; i < 10; i++) {
        display_entity(COLORCODES_FROG_B, (StringArt){.art=_FROG_PLAY_ART, .length=FROG_PLAY_ART_LENGTH}, (Position){map_position.x + (int)(map_width/2), map.sidewalk.y + counter}, (Position){map_position.x + (int)(map_width/2), map.sidewalk.y + FROG_HEIGHT + counter}, map);
        sleep(1);
        counter -= 3;
    }
    wgetch(stdscr);
    
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
