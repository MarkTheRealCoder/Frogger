#include "routines.h"
#include "core.h"

void *example_routine() 
{
    sleepy(1, TIMEFRAME_SECONDS);
    return NULL; 
}

void *example_producer2(void *args)
{
    Packet *packet = (Packet *) args;

    struct game_threads *game = (struct game_threads *) packet->data;

    atomic_int *signal = &game->time.signal;
    pthread_mutex_t *mutex = &game->time.mutex;

    Packet **comms_buffer = (Packet **) game->comms->buffer;
    int buffer_size = game->comms->buffer_size;

    Packet *product;
    int *index = &game->comms->next_prod_index;

    while (true)
    {
        wait_producer(game);

        CHECK_SIGNAL(signal, mutex)
     
        /* generation of the packet with its contents */
        int gen = gen_num(10, 99);
        product = create_packet(&gen, 1, PACKET_TYPE__INT, true);       
        
        /* writing in the communication buffer the created packet. */
        wait_mutex(game);
        comms_buffer[*index] = product;
        printf("Produced\t: %d (index: %d)\n", *(int *) product->data, *index);
        *index = (*index + 1) % buffer_size;
        signal_mutex(game);
        
        signal_producer(game);

        sleepy(50, TIMEFRAME_MILLIS);
    }
    
    printf("exited from prod2\n");
    destroy_packet(packet);

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

    Packet *product;
    int *index = &game->comms->next_prod_index;

    while (true)
    {
        wait_producer(game);

        CHECK_SIGNAL(signal, mutex)
     
        /* generation of the packet with its contents */
        int gen = gen_num(10, 99);
        product = create_packet(&gen, 1, PACKET_TYPE__INT, true);
        
        /* writing in the communication buffer the created packet. */
        wait_mutex(game);
        comms_buffer[*index] = product;
        printf("Produced\t: %d (index: %d)\n", *(int *) product->data, *index);
        *index = (*index + 1) % buffer_size;
        signal_mutex(game);
        
        signal_producer(game);

        sleepy(50, TIMEFRAME_MILLIS);
    }

    printf("exited from prod1\n");
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
        printf("Read\t\t: %d (index: %d)\n", *(int *) consumed_product->data, index);
        index = (index + 1) % buffer_size;
        signal_mutex(game);

        /* destruction of the now-consumed packet. */
        destroy_packet(consumed_product);

        signal_consumer(game);

        sleepy(200, TIMEFRAME_MILLIS);
    }

    printf("exited from master\n");
    destroy_packet(packet);

    return NULL;
}


void set_timer_active(sem_t *status) {
    sem_post(status);
}

void set_timer_stopped(sem_t *status) {
    sem_wait(status);
}

void *run_timer(void *args) {
    Packet *packet = (Packet *) args;

    struct game_threads *game = (struct game_threads *) packet->data;

    atomic_int *signal = &game->time.signal;
    pthread_mutex_t *mutex = &game->time.mutex;

    Packet **comms_buffer = (Packet **) game->comms->buffer;
    int buffer_size = game->comms->buffer_size;

    Packet *product;
    int *index = &game->comms->next_prod_index;

    int gen = CORE_GAME_MANCHE_MAXTIME + CORE_GAME_MANCHE_FRACTION;
    while (true)
    {
        wait_producer(game);

        CHECK_SIGNAL(signal, mutex)
     
        /* generation of the packet with its contents */
        gen -= CORE_GAME_MANCHE_FRACTION;
        product = create_packet(&gen, 1, PACKET_TYPE__INT, true);
        
        /* writing in the communication buffer the created packet. */
        wait_mutex(game);
        comms_buffer[*index] = product;
        printf("Produced Timer\t: %d (index: %d)\n", *(int *) product->data, *index);
        *index = (*index + 1) % buffer_size;
        signal_mutex(game);
        
        signal_producer(game);

        sleepy(CORE_GAME_MANCHE_FRACTION*10, TIMEFRAME_MILLIS);
    }

    printf("exited from prod1\n");
    destroy_packet(packet);

    return NULL;
}































