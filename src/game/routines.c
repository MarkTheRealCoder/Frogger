#include "routines.h"
#include "core.h"

void *example_routine() 
{
    sleepy(5, TIMEFRAME_SECONDS);
    return NULL; 
}

void *example_producer(void *args)
{
    Packet *packet = (Packet *) args;

    struct game_threads *game = (struct game_threads *) packet->data;

    atomic_int *signal = &game->frog.signal;
    pthread_mutex_t *mutex = &game->frog.mutex;

    Packet **comms_buffer = (Packet **) game->comms->buffer;
    int buffer_size = game->comms->buffer_size;

    Packet *produced_product;
    static int index = 0;

    while (true)
    {
        CHECK_SIGNAL(signal, mutex)
     
        /* generation of the packet with its contents */
        int gen = gen_num(100, 999);
        produced_product = create_packet(&gen, 1, PACKET_TYPE__INT, true);

        wait_producer(game);
        
        /* writing in the communication buffer the created packet. */
        wait_mutex(game);
        comms_buffer[index] = produced_product;
        index = (index + 1) % buffer_size;
        signal_mutex(game);

        printf("Produced\t: %d (index: %d)\n", *(int *) produced_product->data, index);
        
        signal_producer(game);

        sleepy(5, TIMEFRAME_MILLIS);
    }

    destroy_packet(packet);

    return NULL;
}

void *master_routine(void *args)
{
    Packet *packet = (Packet *) args;

    struct game_threads *game = (struct game_threads *) packet->data;

    atomic_int *signal = &game->master.signal;
    pthread_mutex_t *mutex = &game->master.mutex;

    Packet **comms_buffer = (Packet **) game->comms->buffer;
    int buffer_size = game->comms->buffer_size;

    Packet *consumed_product;
    int index = 0;

    while (true)
    {
        wait_consumer(game);

        CHECK_SIGNAL(signal, mutex)
        
        /* consumption of the packet from the communication buffer. */
        wait_mutex(game);
        consumed_product = comms_buffer[index];
        index = (index + 1) % buffer_size;
        signal_mutex(game);

        /* destruction of the now-consumed packet. */
        printf("Read\t\t: %d (index: %d)\n", *(int *) consumed_product->data, index);
        destroy_packet(consumed_product);
                
        signal_consumer(game);
    }

    destroy_packet(packet);

    return NULL;
}

