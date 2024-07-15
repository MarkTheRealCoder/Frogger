#include "core.h"
#include "routines.h"

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

void broadcast_signal(struct game_threads *game_threads, GameSignal signal)
{
    printf("- Broadcast of new signal to all threads: %d\n", signal);

    int crocs_num = game_threads->crocs_num;
    int plants_num = game_threads->plants_num;

    atomic_store(&game_threads->master.signal, signal);
    atomic_store(&game_threads->frog.signal, signal);
    atomic_store(&game_threads->time.signal, signal);
    atomic_store(&game_threads->projectile.signal, signal);

    for (int i = 0; i < crocs_num; i++) 
    {
        atomic_store(&game_threads->crocs[i].signal, signal);
    }

    for (int i = 0; i < plants_num; i++) 
    {
        atomic_store(&game_threads->plants[i].signal, signal);
    }
}

void init_signals(struct game_threads *game_threads)
{
    int crocs_num = game_threads->crocs_num;
    int plants_num = game_threads->plants_num;
    
    atomic_init(&game_threads->master.signal, GAMESIGNAL_RUN);
    atomic_init(&game_threads->frog.signal, GAMESIGNAL_RUN);
    atomic_init(&game_threads->time.signal, GAMESIGNAL_RUN);
    atomic_init(&game_threads->projectile.signal, GAMESIGNAL_RUN);
    
    for (int i = 0; i < crocs_num; i++) 
    {
        atomic_init(&game_threads->crocs[i].signal, GAMESIGNAL_RUN);
    }

    for (int i = 0; i < plants_num; i++) 
    {
        atomic_init(&game_threads->plants[i].signal, GAMESIGNAL_RUN);
    }

    init_game_mutexes(game_threads);
    init_semaphores(game_threads);
}

void create_threads(struct game_threads *game_threads)
{
    int crocs_num = game_threads->crocs_num;
    int plants_num = game_threads->plants_num;

    game_threads->crocs = MALLOC(GameThread, crocs_num);
    CRASH_IF_NULL(game_threads->crocs)

    game_threads->plants = MALLOC(GameThread, plants_num);
    CRASH_IF_NULL(game_threads->plants)

    // Initialize signals for threads.
    init_signals(game_threads);
    halt_threads(game_threads); 
    
    Packet *masterPacket = create_packet(game_threads, 1, PACKET_TYPE__GAMETHREADS, false);
    Packet *producerPacket = create_packet(game_threads, 1, PACKET_TYPE__GAMETHREADS, false);

    pthread_create(&game_threads->master.thread, NULL, master_routine, masterPacket);
    pthread_create(&game_threads->frog.thread, NULL, example_producer, producerPacket);
    pthread_create(&game_threads->time.thread, NULL, example_routine, NULL);
    pthread_create(&game_threads->projectile.thread, NULL, example_routine, NULL);

    for (int i = 0; i < crocs_num; i++) 
    {
        pthread_create(&game_threads->crocs[i].thread, NULL, example_routine, NULL);
    }

    for (int i = 0; i < plants_num; i++) 
    {
         pthread_create(&game_threads->plants[i].thread, NULL, example_routine, NULL);
    }
}

void join_threads(struct game_threads *game_threads)
{
    int crocs_num = game_threads->crocs_num;
    int plants_num = game_threads->plants_num;

    pthread_join(game_threads->master.thread, NULL);
    pthread_join(game_threads->frog.thread, NULL);
    pthread_join(game_threads->time.thread, NULL);
    pthread_join(game_threads->projectile.thread, NULL);

    for (int i = 0; i < crocs_num; i++) 
    {
        pthread_join(game_threads->crocs[i].thread, NULL);
    }

    for (int i = 0; i < plants_num; i++) 
    {
        pthread_join(game_threads->plants[i].thread, NULL);
    }
}

/*
 *
 * Signals related.
 *
 */

void run_threads(struct game_threads *game_threads)
{
    broadcast_signal(game_threads, GAMESIGNAL_RUN);
    unlock_game_mutexes(game_threads);
}

void halt_threads(struct game_threads *game_threads)
{
    broadcast_signal(game_threads, GAMESIGNAL_HALT); 
    lock_game_mutexes(game_threads);
}

void stop_threads(struct game_threads *game_threads)
{
    broadcast_signal(game_threads, GAMESIGNAL_STOP);
    unlock_game_mutexes(game_threads);
}

void cancel_threads(struct game_threads *game_threads)
{
    stop_threads(game_threads);

    // In modo da sbloccare il master thread consumatore in caso
    // non ci siano elementi da consumare.
    signal_producer(game_threads);

    join_threads(game_threads);

    destroy_game_mutexes(game_threads);
    destroy_semaphores(game_threads);

    free(game_threads->crocs);
    free(game_threads->plants);

    free(game_threads->comms->buffer);
    free(game_threads->comms);

    printf("shutdown completed!\n");
}

/* 
 * 
 * Signal mutexes related.
 *
 */

void init_game_mutexes(struct game_threads *game_threads)
{
    CREATE_MUTEXES(pthread_mutex_init, game_threads, NULL);
}

void lock_game_mutexes(struct game_threads *game_threads)
{
    HANDLE_MUTEXES(pthread_mutex_lock, game_threads);
}

void unlock_game_mutexes(struct game_threads *game_threads)
{
    HANDLE_MUTEXES(pthread_mutex_unlock, game_threads);
}

void destroy_game_mutexes(struct game_threads *game_threads)
{
    HANDLE_MUTEXES(pthread_mutex_destroy, game_threads);
}

/*
 * 
 * Semaphores & producer/consumer related.
 *
 */

void init_semaphores(struct game_threads *game_threads)
{
    struct comms *comms = game_threads->comms;

    sem_init(&comms->sem_free, 0, comms->buffer_size);
    sem_init(&comms->sem_occupied, 0, 0);
    sem_init(&comms->sem_mutex, 0, 1);
}

void destroy_semaphores(struct game_threads *game_threads)
{
    struct comms *comms = game_threads->comms;

    sem_destroy(&comms->sem_free);
    sem_destroy(&comms->sem_occupied);
    sem_destroy(&comms->sem_mutex);
}

void wait_producer(struct game_threads *game_threads)
{
    sem_wait(&game_threads->comms->sem_free);
}

void signal_producer(struct game_threads *game_threads)
{
    sem_post(&game_threads->comms->sem_occupied);
}

void wait_consumer(struct game_threads *game_threads)
{
    sem_wait(&game_threads->comms->sem_occupied);
}

void signal_consumer(struct game_threads *game_threads)
{
    sem_post(&game_threads->comms->sem_free);
}

void wait_mutex(struct game_threads *game_threads)
{
    sem_wait(&game_threads->comms->sem_mutex);
}

void signal_mutex(struct game_threads *game_threads)
{
    sem_post(&game_threads->comms->sem_mutex);
}

