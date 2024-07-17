#ifndef FROGGER_CORE_H
#define FROGGER_CORE_H

#include "../utils/shortcuts.h"


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

    int total_threads;

    struct comms *comms;
};

struct comms 
{
    void *buffer;
    int buffer_size;

    sem_t sem_free;
    sem_t sem_occupied;

    int await_cleanup;
    int next_prod_index;
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

void cleanup_buffer(struct game_threads *game_threads);

#define APPLY_TO_GAME_ARG(func, game, target, arg)  \
    int crocs_num = game->crocs_num;                \
    int plants_num = game->plants_num;              \
                                                    \
    func(game->master.target, arg);                 \
    func(game->frog.target, arg);                   \
    func(game->time.target, arg);                   \
    func(game->projectile.target, arg);             \
                                                    \
    for (int i = 0; i < crocs_num; i++)             \
    {                                               \
        func(game->crocs[i].target, arg);           \
    }                                               \
                                                    \
    for (int i = 0; i < plants_num; i++)            \
    {                                               \
        func(game->plants[i].target, arg);          \
    }

#define APPLY_TO_GAME_ARG_PTR(func, game, target, arg)  \
    int crocs_num = *game->crocs_num;                   \
    int plants_num = *game->plants_num;                 \
                                                        \
    func(game->master.target, arg);                     \
    func(game->frog.target, arg);                       \
    func(game->time.target, arg);                       \
    func(game->projectile.target, arg);                 \
                                                        \
    for (int i = 0; i < crocs_num; i++)                 \
    {                                                   \
        func(game->crocs[i].target, arg);               \
    }                                                   \
                                                        \
    for (int i = 0; i < plants_num; i++)                \
    {                                                   \
        func(game->plants[i].target, arg);              \
    }

#define APPLY_TO_GAME(func, game, target)   \
    int crocs_num = game->crocs_num;        \
    int plants_num = game->plants_num;      \
                                            \
    func(game->master.target);              \
    func(game->frog.target);                \
    func(game->time.target);                \
    func(game->projectile.target);          \
                                            \
    for (int i = 0; i < crocs_num; i++)     \
    {                                       \
        func(game->crocs[i].target);        \
    }                                       \
                                            \
    for (int i = 0; i < plants_num; i++)    \
    {                                       \
        func(game->plants[i].target);       \
    }

#define APPLY_TO_GAME_PTR(func, game, target)   \
    int crocs_num = *game->crocs_num;           \
    int plants_num = *game->plants_num;         \
                                                \
    func(game->master.target);                  \
    func(game->frog.target);                    \
    func(game->time.target);                    \
    func(game->projectile.target);              \
                                                \
    for (int i = 0; i < crocs_num; i++)         \
    {                                           \
        func(game->crocs[i].target);            \
    }                                           \
                                                \
    for (int i = 0; i < plants_num; i++)        \
    {                                           \
        func(game->plants[i].target);           \
    }

#endif // !FROGGER_CORE_H
