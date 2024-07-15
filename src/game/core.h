#ifndef FROGGER_CORE_H
#define FROGGER_CORE_H

#include "../utils/shortcuts.h"


#define CREATE_MUTEXES(func, mutexes, arg) {                \
    int crocs_num = mutexes->crocs_num;                     \
    int plants_num = mutexes->plants_num;                   \
                                                            \
    func(&mutexes->master.mutex, arg);                      \
    func(&mutexes->frog.mutex, arg);                        \
    func(&mutexes->time.mutex, arg);                        \
    func(&mutexes->projectile.mutex, arg);                  \
                                                            \
    for (int i = 0; i < crocs_num; i++)                     \
    {                                                       \
        func(&mutexes->crocs[i].mutex, arg);                \
    }                                                       \
                                                            \
    for (int i = 0; i < plants_num; i++)                    \
    {                                                       \
        func(&mutexes->plants[i].mutex, arg);               \
    }                                                       \
}

#define HANDLE_MUTEXES(func, mutexes) {                     \
    int crocs_num = mutexes->crocs_num;                     \
    int plants_num = mutexes->plants_num;                   \
                                                            \
    func(&mutexes->master.mutex);                           \
    func(&mutexes->frog.mutex);                             \
    func(&mutexes->time.mutex);                             \
    func(&mutexes->projectile.mutex);                       \
                                                            \
    for (int i = 0; i < crocs_num; i++)                     \
    {                                                       \
        func(&mutexes->crocs[i].mutex);                     \
    }                                                       \
                                                            \
    for (int i = 0; i < plants_num; i++)                    \
    {                                                       \
        func(&mutexes->plants[i].mutex);                    \
    }                                                       \
}


/*
 * Packets related.
 */

typedef enum
{
    PACKET_TYPE__VOID,
    PACKET_TYPE__INT,
    PACKET_TYPE__GAMETHREADS
} PacketType;

typedef struct
{
    void *data;
    PacketType type;
    bool cloned;
} Packet;

Packet *create_packet(void *data, int size, PacketType packetType, bool clone);
void destroy_packet(Packet *packet);


/*
 * Game threads & signals related.
 */

typedef enum 
{
    GAMESIGNAL_RUN,
    GAMESIGNAL_HALT,
    GAMESIGNAL_STOP
} GameSignal;

typedef struct
{
    pthread_t thread;
    atomic_int signal;
    pthread_mutex_t mutex;
} GameThread;

struct game_threads
{
    GameThread master;
    GameThread frog;
    GameThread time;
    GameThread projectile;

    GameThread *crocs;
    GameThread *plants;
    int crocs_num;
    int plants_num;

    struct comms *comms;
};

struct comms 
{
    void *buffer;
    int buffer_size;

    sem_t sem_free;
    sem_t sem_occupied;
    sem_t sem_mutex;
};

void create_threads(struct game_threads *game_threads);

void join_threads(struct game_threads *game_threads);
void run_threads(struct game_threads *game_threads);
void halt_threads(struct game_threads *game_threads);
void cancel_threads(struct game_threads *game_threads);


void init_game_mutexes(struct game_threads *game_threads);
void lock_game_mutexes(struct game_threads *game_threads);
void unlock_game_mutexes(struct game_threads *game_threads);
void destroy_game_mutexes(struct game_threads *game_threads);


void init_semaphores(struct game_threads *game_threads);
void destroy_semaphores(struct game_threads *game_threads);

void wait_producer(struct game_threads *game_threads);
void signal_producer(struct game_threads *game_threads);

void wait_consumer(struct game_threads *game_threads);
void signal_consumer(struct game_threads *game_threads);

void wait_mutex(struct game_threads *game_threads);
void signal_mutex(struct game_threads *game_threads);

#endif // !FROGGER_CORE_H
