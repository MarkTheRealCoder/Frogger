#include "game/core.h"
#include "game/addons.h"
#include "utils/shortcuts.h"

#define TEST_MODE true

void test_packets();
void test_threads(struct game_threads *game_threads);

int main(int argc, char *argv[])
{
    srand(time(NULL));
    struct program_args parsed_program_args = parse_program_args(argc, argv); 

    if (parsed_program_args.help)
    {
        program_args_print_help(); 
        return EXIT_SUCCESS;
    }

    struct game_threads game_threads = { };
    game_threads.crocs_num = 2;
    game_threads.plants_num = 2;

    game_threads.comms = MALLOC(struct comms, 1);
    game_threads.comms->buffer = MALLOC(Packet *, 10);
    game_threads.comms->buffer_size = 10;
    game_threads.comms->next_prod_index = 0;
    game_threads.total_threads = 4 + game_threads.crocs_num + game_threads.plants_num;

    if (TEST_MODE)
    {
        test_packets();
        test_threads(&game_threads);
        return EXIT_SUCCESS;
    }
    
    // da eseguire all'inizio di ogni partita
    create_threads(&game_threads);

    // da eseguire alla pausa della partita
    halt_threads(&game_threads);

    // da eseguire alla fine di ogni partita
    cancel_threads(&game_threads);

    // da eseguire alla fine di ogni partita
    if (parsed_program_args.save_game_stats)
    {
        // commentato per evitare la creazione del file
        // program_args_save_stats();
    }

    // da eseguire alla fine di ogni partita, dopo il save_game_stats
    if (parsed_program_args.quit_on_win)
    {
        cancel_threads(&game_threads); 
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}

void test_packets()
{
    int num = 300;
    Packet *packet;

    printf("\ncloned:\n");
    packet = create_packet(&num, PACKET_TYPE__INT, 1, true);
    printf("\torigin: %d | packet: %d\n", num, *((int *) packet->data));
    num = 0;
    printf("\torigin: %d | packet: %d\n", num, *((int *) packet->data));
    destroy_packet(packet);

    num = 300;
    printf("not cloned:\n");
    packet = create_packet(&num, PACKET_TYPE__INT, 1, false);
    printf("\torigin: %d | packet: %d\n", num, *((int *) packet->data));
    num = 0;
    printf("\torigin: %d | packet: %d\n\n", num, *((int *) packet->data));
    destroy_packet(packet);
}

void test_threads(struct game_threads *game_threads)
{
    create_threads(game_threads);
    sleepy(500, TIMEFRAME_MILLIS);
    run_threads(game_threads);
    sleepy(500, TIMEFRAME_MILLIS);
    halt_threads(game_threads);
    sleepy(500, TIMEFRAME_MILLIS);
    run_threads(game_threads);
    sleepy(500, TIMEFRAME_MILLIS);
    cancel_threads(game_threads);
}
