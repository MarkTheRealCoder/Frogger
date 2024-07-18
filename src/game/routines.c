#include "routines.h"
#include "core.h"

/*
 * Restituisce la stringa corrispondente al tipo di pacchetto.
 * @param packetType    Il tipo di pacchetto.
 * @return              La stringa corrispondente al tipo di pacchetto.
 */
char *str_packet_type(PacketType packetType)
{
    switch (packetType)
    {
        case PACKET_TYPE__INT:
            return "INT";
        case PACKET_TYPE__GAMETHREADS:
            return "GAMETHREADS";
        case PACKET_TYPE__TIMER:
            return "TIMER";
        default:
            return "VOID";
    }
}

void *example_routine() 
{
    sleepy(1, TIMEFRAME_SECONDS);
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

        DEBUG("Produced\t: index: %d\ttype: %s\n", *index, str_packet_type(product->type));
        
        *index = (*index + 1) % buffer_size;
        signal_mutex(game);
        
        signal_producer(game);

        sleepy(50, TIMEFRAME_MILLIS);
    }

    DEBUG("exited from prod1\n");

    return NULL;
}

/*
 * La routine dedicata alla gestione del gioco.
 * @param args  Il pacchetto contenente i dati del gioco.
 */
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

        DEBUG("Read\t\t: index: %d\ttype: %s\n", index - 1, str_packet_type(consumed_product->type))
        
        /* destruction of the now-consumed packet. */
        destroy_packet(consumed_product);

        signal_consumer(game);
    }

    DEBUG("exited from master\n");

    return NULL;
}

/*
 * La routine dedicata alla modifica e comunicazione del tempo di gioco.
 * @param args  Il pacchetto contenente i dati del gioco.
 */
void *run_timer(void *args) 
{
    Packet *packet = (Packet *) args;

    struct game_threads *game = (struct game_threads *) packet->data;

    atomic_int *signal = &game->time.signal;
    pthread_mutex_t *mutex = &game->time.mutex;

    Packet **comms_buffer = (Packet **) game->comms->buffer;
    int buffer_size = game->comms->buffer_size;

    Packet *product;
    int *index = &game->comms->next_prod_index;

    TimerPacket timer_packet = { };
    timer_packet.current_time = CORE_GAME_MANCHE_MAXTIME;
    timer_packet.max_time = CORE_GAME_MANCHE_MAXTIME;

    while (true)
    {
        wait_producer(game);

        CHECK_SIGNAL(signal, mutex)
     
        /* generation of the packet with its contents */
        timer_packet.current_time -= CORE_GAME_MANCHE_FRACTION;
        product = create_packet(&timer_packet, 1, PACKET_TYPE__TIMER, true);
        
        /* writing in the communication buffer the created packet. */
        wait_mutex(game);
        comms_buffer[*index] = product;

        DEBUG("Produced\t: index: %d\ttype: %s\n", *index, str_packet_type(product->type));

        *index = (*index + 1) % buffer_size;
        signal_mutex(game);
        
        signal_producer(game);

        sleepy(CORE_GAME_MANCHE_FRACTION, TIMEFRAME_SECONDS);
    }

    DEBUG("exited from timer\n");

    return NULL;
}

