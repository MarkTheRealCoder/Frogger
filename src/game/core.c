#include "core.h"
#include "routines.h"

void init_game_threads(struct game_threads *game_threads)
{
    game_threads->crocs_num = 2;
    game_threads->plants_num = 2;
    game_threads->total_threads = CORE_THREADS + game_threads->crocs_num + game_threads->plants_num;

    game_threads->comms = MALLOC(struct comms, 1);
    game_threads->comms->buffer = MALLOC(Packet *, CORE_BUFFER_SIZE);
    game_threads->comms->buffer_size = CORE_BUFFER_SIZE;
    game_threads->comms->next_prod_index = 0;
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

void broadcast_signal(struct game_threads *game_threads, GameSignal newSignal)
{
    printf("- Broadcast of new signal to all threads: %d\n", newSignal);
    APPLY_TO_GAME_ARG_PTR(atomic_store, &game_threads, signal, newSignal)
}

void init_signals(struct game_threads *game_threads)
{
    APPLY_TO_GAME_ARG_PTR(atomic_init, &game_threads, signal, GAMESIGNAL_RUN) 

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

    init_signals(game_threads);
    halt_threads(game_threads); 
    
    Packet *masterPacket = create_packet(game_threads, 1, PACKET_TYPE__GAMETHREADS, false);
    Packet *producerPacket = create_packet(game_threads, 1, PACKET_TYPE__GAMETHREADS, false);
    Packet *producerPacket2 = create_packet(game_threads, 1, PACKET_TYPE__GAMETHREADS, false);

    pthread_create(&game_threads->master.thread, NULL, master_routine, masterPacket);
    pthread_create(&game_threads->frog.thread, NULL, example_producer, producerPacket);
    pthread_create(&game_threads->time.thread, NULL, run_timer, producerPacket2);
    pthread_create(&game_threads->plants_projectile.thread, NULL, example_routine, NULL);
    pthread_create(&game_threads->frog_projectile.thread, NULL, example_routine, NULL);

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
    APPLY_TO_GAME_ARG(pthread_join, game_threads, thread, NULL)
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

    // In modo da sbloccare i thread produttori in caso
    // non ci siano elementi da produrre.
    for (int i = 0; i < game_threads->total_threads; i++) 
    {
        signal_consumer(game_threads);
    }

    join_threads(game_threads);
    
    cleanup_buffer(game_threads);

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
    APPLY_TO_GAME_ARG_PTR(pthread_mutex_init, &game_threads, mutex, NULL)
}

void lock_game_mutexes(struct game_threads *game_threads)
{
    APPLY_TO_GAME_PTR(pthread_mutex_lock, &game_threads, mutex)
}

void unlock_game_mutexes(struct game_threads *game_threads)
{
    APPLY_TO_GAME_PTR(pthread_mutex_unlock, &game_threads, mutex)
}

void destroy_game_mutexes(struct game_threads *game_threads)
{
    APPLY_TO_GAME_PTR(pthread_mutex_destroy, &game_threads, mutex)
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

int await_cleanup_count(struct game_threads *game_threads)
{
    int count;
    sem_getvalue(&game_threads->comms->sem_occupied, &count);
    return count;
}

void cleanup_buffer(struct game_threads *game_threads)
{
    printf("still awaiting cleanup: %d elems\n", await_cleanup_count(game_threads));
    
    Packet **comms_buffer = (Packet **) game_threads->comms->buffer;
    int buffer_size = game_threads->comms->buffer_size;

    wait_mutex(game_threads);

    int await_cleanup = await_cleanup_count(game_threads);
    int next_prod_index = game_threads->comms->next_prod_index;
    
    for (int i = 0; i < await_cleanup; i++) 
    {
        int pos = (next_prod_index - 1 - i + buffer_size) % buffer_size;
     
        printf("clean buffer(%d) = ", pos);
        printf("%d\n", *(int *)comms_buffer[pos]->data);

        destroy_packet(comms_buffer[pos]);
    }
    
    signal_mutex(game_threads);
}



