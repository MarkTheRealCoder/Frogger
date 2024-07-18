#include "core.h"
#include "routines.h"

/**
 * Inizializza la struttura game_threads.
 * @param game_threads  La struttura da inizializzare.
 */
void init_game_threads(struct game_threads *game)
{
    game->crocs_num = 2;
    game->plants_num = 2;
    game->total_threads = CORE_THREADS + game->crocs_num + game->plants_num;

    game->comms = MALLOC(struct comms, 1);
    game->comms->buffer = MALLOC(Packet *, CORE_BUFFER_SIZE);
    game->comms->buffer_size = CORE_BUFFER_SIZE;
    game->comms->next_prod_index = 0;
}

int calculate_manche_points(int frog_lives, int plants_alive, int time_remaining)
{
    int points = 0;

    points = (frog_lives * 2) + time_remaining;
    points = plants_alive >= points ? 1 : points - plants_alive;

    return points;
}

Packet *create_packet(void *data, int size, PacketType packetType, bool clone)
{
    Packet *packet = MALLOC(Packet, 1); 
    CRASH_IF_NULL(packet)

    switch (packetType) 
    {
        case PACKET_TYPE__INT:
            {
                ALLOC_PACKET_DATA(data, packet->data, int, size, clone)
                break;
            }
        case PACKET_TYPE__GAMETHREADS:
            {
                ALLOC_PACKET_DATA(data, packet->data, GameThread, size, clone)
                break;
            }
        case PACKET_TYPE__TIMER:
            {
                ALLOC_PACKET_DATA(data, packet->data, TimerPacket, size, clone)
                break;
            }
        default:
            {
                ALLOC_PACKET_DATA(data, packet->data, void, size, clone)
                break;
            }
    }

    packet->type = packetType;
    packet->cloned = clone;
    
    return packet;
}

void destroy_packet(Packet *packet)
{
    if (packet->cloned)
    {
        free(packet->data);
    }

    free(packet);
}

void broadcast_signal(struct game_threads *game, GameSignal newSignal)
{
    DEBUG("- Broadcast of new signal to all threads: %d\n", newSignal);
    APPLY_TO_GAME_ARG_PTR(atomic_store, &game, signal, newSignal)
}

void init_signals(struct game_threads *game)
{
    APPLY_TO_GAME_ARG_PTR(atomic_init, &game, signal, GAMESIGNAL_RUN) 

    init_game_mutexes(game);
    init_semaphores(game);
}

void create_threads(struct game_threads *game)
{
    int crocs_num = game->crocs_num;
    int plants_num = game->plants_num;

    game->crocs = MALLOC(GameThread, crocs_num);
    CRASH_IF_NULL(game->crocs)

    game->plants = MALLOC(GameThread, plants_num);
    CRASH_IF_NULL(game->plants)

    init_signals(game);
    halt_threads(game); 
    
    Packet *masterPacket = create_packet(game, 1, PACKET_TYPE__GAMETHREADS, false);
    Packet *producerPacket = create_packet(game, 1, PACKET_TYPE__GAMETHREADS, false);
    Packet *producerPacket2 = create_packet(game, 1, PACKET_TYPE__GAMETHREADS, false);

    pthread_create(&game->master.thread, NULL, master_routine, masterPacket);
    pthread_create(&game->frog.thread, NULL, example_producer, producerPacket);
    pthread_create(&game->time.thread, NULL, run_timer, producerPacket2);
    pthread_create(&game->plants_projectile.thread, NULL, example_routine, NULL);
    pthread_create(&game->frog_projectile.thread, NULL, example_routine, NULL);

    for (int i = 0; i < crocs_num; i++) 
    {
        pthread_create(&game->crocs[i].thread, NULL, example_routine, NULL);
    }

    for (int i = 0; i < plants_num; i++) 
    {
         pthread_create(&game->plants[i].thread, NULL, example_routine, NULL);
    }
}

void join_threads(struct game_threads *game)
{
    APPLY_TO_GAME_ARG(pthread_join, game, thread, NULL)
}

/*
 *
 * Signals related.
 *
 */

void run_threads(struct game_threads *game)
{
    broadcast_signal(game, GAMESIGNAL_RUN);
    unlock_game_mutexes(game);
}

void halt_threads(struct game_threads *game)
{
    broadcast_signal(game, GAMESIGNAL_HALT); 
    lock_game_mutexes(game);
}

void stop_threads(struct game_threads *game)
{
    broadcast_signal(game, GAMESIGNAL_STOP);
    unlock_game_mutexes(game);
}

void cancel_threads(struct game_threads *game)
{
    stop_threads(game);

    // In modo da sbloccare il master thread consumatore in caso
    // non ci siano elementi da consumare.
    signal_producer(game);

    // In modo da sbloccare i thread produttori in caso
    // non ci siano elementi da produrre.
    for (int i = 0; i < game->total_threads; i++) 
    {
        signal_consumer(game);
    }

    join_threads(game);
    
    cleanup_buffer(game);

    destroy_game_mutexes(game);
    destroy_semaphores(game);

    free(game->crocs);
    free(game->plants);

    free(game->comms->buffer);
    free(game->comms);

    DEBUG("shutdown completed!\n");
}

/* 
 * 
 * Signal mutexes related.
 *
 */

void init_game_mutexes(struct game_threads *game)
{
    APPLY_TO_GAME_ARG_PTR(pthread_mutex_init, &game, mutex, NULL)
}

void lock_game_mutexes(struct game_threads *game)
{
    APPLY_TO_GAME_PTR(pthread_mutex_lock, &game, mutex)
}

void unlock_game_mutexes(struct game_threads *game)
{
    APPLY_TO_GAME_PTR(pthread_mutex_unlock, &game, mutex)
}

void destroy_game_mutexes(struct game_threads *game)
{
    APPLY_TO_GAME_PTR(pthread_mutex_destroy, &game, mutex)
}

/*
 * 
 * Semaphores & producer/consumer related.
 *
 */

void init_semaphores(struct game_threads *game)
{
    struct comms *comms = game->comms;

    sem_init(&comms->sem_free, 0, comms->buffer_size);
    sem_init(&comms->sem_occupied, 0, 0);
    sem_init(&comms->sem_mutex, 0, 1);
}

void destroy_semaphores(struct game_threads *game)
{
    struct comms *comms = game->comms;

    sem_destroy(&comms->sem_free);
    sem_destroy(&comms->sem_occupied);
    sem_destroy(&comms->sem_mutex);
}

void wait_producer(struct game_threads *game)
{
    sem_wait(&game->comms->sem_free);
}

void signal_producer(struct game_threads *game)
{
    sem_post(&game->comms->sem_occupied);
}

void wait_consumer(struct game_threads *game)
{
    sem_wait(&game->comms->sem_occupied);
}

void signal_consumer(struct game_threads *game)
{
    sem_post(&game->comms->sem_free);
}

void wait_mutex(struct game_threads *game)
{
    sem_wait(&game->comms->sem_mutex);
}

void signal_mutex(struct game_threads *game)
{
    sem_post(&game->comms->sem_mutex);
}

int await_cleanup_count(struct game_threads *game)
{
    int count;
    sem_getvalue(&game->comms->sem_occupied, &count);
    return count;
}

void cleanup_buffer(struct game_threads *game)
{
    DEBUG("still awaiting cleanup: %d elems\n", await_cleanup_count(game));
    
    Packet **comms_buffer = (Packet **) game->comms->buffer;
    int buffer_size = game->comms->buffer_size;

    wait_mutex(game);

    int await_cleanup = await_cleanup_count(game);
    int next_prod_index = game->comms->next_prod_index;
    
    for (int i = 0; i < await_cleanup; i++) 
    {
        int pos = (next_prod_index - 1 - i + buffer_size) % buffer_size;
        destroy_packet(comms_buffer[pos]);

        DEBUG("cleaned buffer[%d]\n", pos);
    }
    
    signal_mutex(game);
}

