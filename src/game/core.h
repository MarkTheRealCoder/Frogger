#ifndef FROGGER_CORE_H
#define FROGGER_CORE_H

#include "../utils/imports.h"
#include "entities.h"

// Rispettivamente: master, frog, time, frog_projectile, plants_projectile threads.
#define CORE_THREADS 5 
#define CORE_BUFFER_SIZE 64

#define CORE_GAME_HIDEOUTS 5
#define CORE_GAME_HIDEOUTS_SPACES_INBETWEEN 4
#define CORE_GAME_MANCHE_MAXTIME 120 // in secondi
#define CORE_GAME_MANCHE_FRACTION 1 // in secondi

#define CORE_GAME_ENTITY_SIZE 3

#define CORE_GAME_FROG_LIVES 5
#define CORE_GAME_FROG_JUMP_X 1
#define CORE_GAME_FROG_JUMP_Y CORE_GAME_ENTITY_SIZE

#define CORE_GAME_LAWN_TOP_LANES 2
#define CORE_GAME_LAWN_BOTTOM_LANES 2
#define CORE_GAME_RIVER_LANES 8

#define CORE_GAME_MAP_WIDTH                                         \
    ((CORE_GAME_HIDEOUTS * CORE_GAME_ENTITY_SIZE) +                 \
    (CORE_GAME_HIDEOUTS_SPACES_INBETWEEN * CORE_GAME_ENTITY_SIZE) + \
    (2 * CORE_GAME_ENTITY_SIZE))

#define CORE_GAME_PROJECTILE_WAIT 2 // in secondi

#define CORE_GAME_PLANTS 3
#define CORE_GAME_CROCS (2 * CORE_GAME_RIVER_LANES)

#define CORE_GAME_CROCS_MIN_WIDTH (2 * CORE_GAME_ENTITY_SIZE)
#define CORE_GAME_CROCS_MAX_WIDTH (3 * CORE_GAME_ENTITY_SIZE)

/**
 * Game Core Structures
 */

typedef struct {
    Position current;
    Position last;
    EntityType type;
    TrueType trueType;
    int hps;
    int width;
    int height;
    bool readyToShoot;
} Entity;

struct entities_list {
    Entity e;
    struct entities_list *next;
};

typedef struct {
    struct entities_list *entities;
    int entity_num;
} Entities;

typedef struct {
    enum {CLOCK_MAIN, CLOCK_SECONDARY} type;
    unsigned int starting;
    int current;
    int fraction;
} Clock;

typedef struct {
    void *component;
    enum {COMPONENT_ENTITY, COMPONENT_ENTITIES, COMPONENT_CLOCK} type;
} Component;

typedef struct {
    int current_plants;
    int current_projectiles;
    int current_frog_projectiles;
    Component components[MAX_CONCURRENCY];
    MapSkeleton map;
    StringList *achievements;
} GameSkeleton;

Component *find_component(int index, GameSkeleton *game);
void update_position (Entity *e, Action movement);

/**
 * Packets related.
 */

typedef enum
{
    PACKET_TYPE__VOID,
    PACKET_TYPE__INT,
    PACKET_TYPE__TIMER,
    PACKET_TYPE__GAMETHREADS,
    PACKET_TYPE__ENTITYMOVE
} PacketType;

typedef struct
{
    int id;
    void *data;
    PacketType type;
    bool cloned;
} Packet_;

Packet_ *create_packet(void *data, int size, PacketType packetType, bool clone);
void destroy_packet(Packet_ *packet);

typedef struct
{
    unsigned int current_time;
    unsigned int max_time;
} TimerPacket;

typedef struct 
{
    struct entity entity;
} EntityMovePacket;

/**
 * Game threads & signals related.
 */
/*
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

    GameThread *crocs;
    GameThread *plants;
    int crocs_num;
    int plants_num;

    GameThread frog_projectile;
    GameThread plants_projectile;

    int total_threads;

    struct entity_node *entity_node; 
    struct comms *comms;

    StringList *packet_logs;
    StringList *achievements;

    MapSkeleton map;
};
struct comms 
{
    void *buffer;
    int buffer_size;

    sem_t sem_free;
    sem_t sem_occupied;

    int next_prod_index;
    sem_t sem_mutex;
}; */

void init_game_threads(struct game_threads *game_threads);
void init_comms(struct game_threads *game, int size);
void init_entity_node(struct game_threads *game);

struct entity_node *entity_node_create();
void entity_node_insert(struct entity_node *head, struct entity entity);
void entity_node_destroy(struct entity_node *entity_node);
void debug_entity_buffer(struct entity_node *entityBuffer);
struct entity *entity_node_find_id(struct entity_node *head, int id);

Packet *create_threads(struct game_threads *game);

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

int await_cleanup_count(struct game_threads *game_threads);
void cleanup_comms_buffer(struct game_threads *game);

void setup_map(GameSkeleton *game);

#define CASE_PACKET_ALLOC(structure, type, from, to, size, clone)   \
    case type:                                                      \
        ALLOC_PACKET_DATA(from, to, structure, size, clone)         \
        break;

#define APPLY_TO_GAME_ARG(func, game, target, arg)  \
    int crocs_num = game->crocs_num;                \
    int plants_num = game->plants_num;              \
                                                    \
    func(game->master.target, arg);                 \
    func(game->frog.target, arg);                   \
    func(game->time.target, arg);                   \
    func(game->frog_projectile.target, arg);        \
    func(game->plants_projectile.target, arg);      \
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
    func(game->frog_projectile.target, arg);            \
    func(game->plants_projectile.target, arg);          \
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

#define APPLY_TO_GAME(func, game, target)       \
    int crocs_num = game->crocs_num;            \
    int plants_num = game->plants_num;          \
                                                \
    func(game->master.target);                  \
    func(game->frog.target);                    \
    func(game->time.target);                    \
    func(game->frog_projectile.target);         \
    func(game->plants_projectile.target);       \
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

#define APPLY_TO_GAME_PTR(func, game, target)   \
    int crocs_num = *game->crocs_num;           \
    int plants_num = *game->plants_num;         \
                                                \
    func(game->master.target);                  \
    func(game->frog.target);                    \
    func(game->time.target);                    \
    func(game->frog_projectile.target);         \
    func(game->plants_projectile.target);       \
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

