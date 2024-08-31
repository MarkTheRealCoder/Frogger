#include "core.h"
#include "routines.h"
#include "../utils/shortcuts.h"

void debug_entity_buffer(struct entity_node *entityBuffer)
{
    struct entity_node *currentEntity = entityBuffer;

    while (currentEntity != NULL)
    {
        DEBUG("Entity: %d\n", currentEntity->entity.id);
        currentEntity = currentEntity->next;
    }
}

/**
* Inizializza la struttura game_threads.
 * @param game_threads  La struttura da inizializzare.
 */
void init_game_threads(struct game_threads *game)
{
    game->crocs_num = 2;
    game->plants_num = 2;
    game->total_threads = CORE_THREADS + game->crocs_num + game->plants_num;

    init_entity_node(game);
    init_comms(game, CORE_BUFFER_SIZE);

    game->achievements = MALLOC(StringList, 1);
    CRASH_IF_NULL(game->achievements)

    game->packet_logs = MALLOC(StringList, 1);
    CRASH_IF_NULL(game->packet_logs)
    
    game->achievements->nodes = 25;
    game->packet_logs->nodes = 25;
}

/**
 * Inizializza la struttura comms.
 * @param game  La struttura game_threads.
 * @param size  La dimensione del buffer di comunicazione da creare.
 */
void init_comms(struct game_threads *game, int size)
{
    struct comms *comms = MALLOC(struct comms, 1);
    CRASH_IF_NULL(comms)

    comms->buffer = MALLOC(Packet *, size);
    CRASH_IF_NULL(comms->buffer)
    
    comms->buffer_size = size;
    comms->next_prod_index = 0;

    game->comms = comms; 
}

void init_entity_node(struct game_threads *game)
{
    struct entity_node *buffer = entity_node_create();
    game->entity_node = buffer;

    int index = 0;

    struct entity frog = entities_default_frog(&index);
    entity_node_insert(buffer, frog);

    for (int i = 0; i < CORE_GAME_PLANTS; i++) 
    {
        struct entity plant = entities_default_plant(&index);
        entity_node_insert(buffer, plant);
    }
    
    for (int i = 0; i < CORE_GAME_CROCS; i++) 
    {
        struct entity croc = entities_default_croc(&index);
        entity_node_insert(buffer, croc);
    }

    debug_entity_buffer(buffer);
}

struct entity_node *entity_node_create()
{
    struct entity_node *new_entity_node = MALLOC(struct entity_node, 1);
    CRASH_IF_NULL(new_entity_node)

    new_entity_node->entity = (struct entity) { .id = -1 };
    new_entity_node->next = NULL;

    return new_entity_node;
}

void entity_node_insert(struct entity_node *head, struct entity entity)
{
    if (head == NULL)
    {
        head = entity_node_create();
    }

    if (head->entity.id == -1)
    {
        head->entity = entity;
        return;
    }

    struct entity_node *new_entity_node = entity_node_create();
    
    while (head->next != NULL) 
    {
        head = head->next;
    }
    
    head->next = new_entity_node;
    new_entity_node->entity = entity;
}

void entity_node_destroy(struct entity_node *head)
{
    struct entity_node *current = head;
    struct entity_node *next = NULL;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
}

struct entity *entity_node_find_id(struct entity_node *head, int id)
{
    struct entity_node *temp = head;

    while (temp != NULL) 
    {
        if (temp->entity.id == id) 
        {
            return &temp->entity;
        }

        temp = temp->next;
    }

    return NULL;
}

/*
 * Calcola i punti della manche.
 * @param frog_lives        Le vite della rana.
 * @param plants_alive      Le piante ancora vive.
 * @param time_remaining    Il tempo rimanente.
 * @return                  I punti della manche.
 */
int calculate_manche_points(int frog_lives, int plants_alive, int time_remaining)
{
    int points = 0;

    points = (frog_lives * 2) + time_remaining;
    points = plants_alive >= points ? 1 : points - plants_alive;

    return points;
}

/*
 * Crea un pacchetto.
 * @param data          I dati da inserire nel pacchetto.
 * @param size          La dimensione dei dati.
 * @param packetType    Il tipo del pacchetto.
 * @param clone         Se i dati devono essere clonati.
 * @return              Il pacchetto creato.
 */
Packet *create_packet(void *data, int size, PacketType packetType, bool clone)
{
    Packet *packet = MALLOC(Packet, 1); 
    CRASH_IF_NULL(packet)

    switch (packetType) 
    {
        default:
            ALLOC_PACKET_DATA(data, packet->data, void, size, clone)
            break;
        
        CASE_PACKET_ALLOC(int,              PACKET_TYPE__INT,           data, packet->data, size, clone)
        CASE_PACKET_ALLOC(TimerPacket,      PACKET_TYPE__TIMER,         data, packet->data, size, clone)
        CASE_PACKET_ALLOC(GameThread,       PACKET_TYPE__GAMETHREADS,   data, packet->data, size, clone)
        CASE_PACKET_ALLOC(EntityMovePacket, PACKET_TYPE__ENTITYMOVE,    data, packet->data, size, clone)
    }

    packet->type = packetType;
    packet->cloned = clone;
    
    return packet;
}

/*
 * Distrugge un pacchetto.
 * @param packet    Il pacchetto da distruggere.
 */
void destroy_packet(Packet *packet)
{
    if (packet->cloned)
    {
        free(packet->data);
    }

    free(packet);
}

/*
 * Segnala un nuovo segnale a tutti i thread.
 * @param game          La struttura game_threads.
 * @param newSignal     Il nuovo segnale.
 */
void broadcast_signal(struct game_threads *game, GameSignal newSignal)
{
    DEBUG("- Broadcast of new signal to all threads: %d\n", newSignal);
    APPLY_TO_GAME_ARG_PTR(atomic_store, &game, signal, newSignal)
}

/*
 * Inizializza i segnali.
 * @param game  La struttura game_threads.
 */
void init_signals(struct game_threads *game)
{
    APPLY_TO_GAME_ARG_PTR(atomic_init, &game, signal, GAMESIGNAL_RUN) 

    init_game_mutexes(game);
    init_semaphores(game);
}

/*
 * Crea i thread di gioco.
 * @param game  La struttura game_threads.
 * @return      Il pacchetto iniziale.
 */
Packet *create_threads(struct game_threads *game)
{
    Packet *packet = create_packet(game, 1, PACKET_TYPE__GAMETHREADS, false);

    int crocs_num = game->crocs_num;
    int plants_num = game->plants_num;

    game->crocs = MALLOC(GameThread, crocs_num);
    CRASH_IF_NULL(game->crocs)

    game->plants = MALLOC(GameThread, plants_num);
    CRASH_IF_NULL(game->plants)

    init_signals(game);
    halt_threads(game);

    pthread_create(&game->master.thread, NULL, master_routine, packet);
    pthread_create(&game->frog.thread, NULL, frog_routine, packet);
    pthread_create(&game->time.thread, NULL, timer_routine, packet);
    pthread_create(&game->plants_projectile.thread, NULL, example_routine, packet);
    pthread_create(&game->frog_projectile.thread, NULL, frog_projectile_routine, packet);

    for (int i = 0; i < crocs_num; i++) 
    {
        pthread_create(&game->crocs[i].thread, NULL, example_routine, packet);
    }

    for (int i = 0; i < plants_num; i++) 
    {
         pthread_create(&game->plants[i].thread, NULL, example_routine, packet);
    }

    return packet; 
}

/*
 * Esegue il JOIN dei thread di gioco.
 * @param game  La struttura game_threads.
 */
void join_threads(struct game_threads *game)
{
    APPLY_TO_GAME_ARG(pthread_join, game, thread, NULL)
}

/*
 *
 * Signals related.
 *
 */

/*
 * Segnala ai thread di riprendere l'esecuzione.
 * @param game  La struttura game_threads.
 */
void run_threads(struct game_threads *game)
{
    broadcast_signal(game, GAMESIGNAL_RUN);
    unlock_game_mutexes(game);
}

/*
 * Segnala ai thread di fermare temporaneamente l'esecuzione.
 * @param game  La struttura game_threads.
 */
void halt_threads(struct game_threads *game)
{
    broadcast_signal(game, GAMESIGNAL_HALT); 
    lock_game_mutexes(game);
}

/*
 * Segnala ai thread di fermare definitivamente l'esecuzione.
 * @param game  La struttura game_threads.
 */
void stop_threads(struct game_threads *game)
{
    broadcast_signal(game, GAMESIGNAL_STOP);
    unlock_game_mutexes(game);
}

/*
 * Cancella i thread di gioco.
 * @param game  La struttura game_threads.
 */
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

    cleanup_comms_buffer(game);
    free(game->comms->buffer);
    free(game->comms);

    destroy_game_mutexes(game);
    destroy_semaphores(game);

    free(game->crocs);
    free(game->plants);

    entity_node_destroy(game->entity_node);
    
    DEBUG("shutdown completed!\n");
}

/* 
 * 
 * Signal mutexes related.
 *
 */

/*
 * Inizializza i mutex dei segnali.
 * @param game  La struttura game_threads.
 */
void init_game_mutexes(struct game_threads *game)
{
    APPLY_TO_GAME_ARG_PTR(pthread_mutex_init, &game, mutex, NULL)
}

/*
 * Blocca i mutex dei segnali.
 * @param game  La struttura game_threads.
 */
void lock_game_mutexes(struct game_threads *game)
{
    APPLY_TO_GAME_PTR(pthread_mutex_lock, &game, mutex)
}

/*
 * Sblocca i mutex dei segnali.
 * @param game  La struttura game_threads.
 */
void unlock_game_mutexes(struct game_threads *game)
{
    APPLY_TO_GAME_PTR(pthread_mutex_unlock, &game, mutex)
}

/*
 * Distrugge i mutex dei segnali.
 * @param game  La struttura game_threads.
 */
void destroy_game_mutexes(struct game_threads *game)
{
    APPLY_TO_GAME_PTR(pthread_mutex_destroy, &game, mutex)
}

/*
 * 
 * Semaphores & producer/consumer related.
 *
 */

/*
 * Inizializza i semafori.
 * @param game  La struttura game_threads.
 */
void init_semaphores(struct game_threads *game)
{
    struct comms *comms = game->comms;

    sem_init(&comms->sem_free, 0, comms->buffer_size);
    sem_init(&comms->sem_occupied, 0, 0);
    sem_init(&comms->sem_mutex, 0, 1);
}

/*
 * Distrugge i semafori.
 * @param game  La struttura game_threads.
 */
void destroy_semaphores(struct game_threads *game)
{
    struct comms *comms = game->comms;

    sem_destroy(&comms->sem_free);
    sem_destroy(&comms->sem_occupied);
    sem_destroy(&comms->sem_mutex);
}

/*
 * Esegue la wait del produttore.
 * @param game  La struttura game_threads.
 */
void wait_producer(struct game_threads *game)
{
    sem_wait(&game->comms->sem_free);
}

/*
 * Esegue la signal del produttore.
 * @param game  La struttura game_threads.
 */
void signal_producer(struct game_threads *game)
{
    sem_post(&game->comms->sem_occupied);
}

/*
 * Esegue la wait del consumatore.
 * @param game  La struttura game_threads.
 */
void wait_consumer(struct game_threads *game)
{
    sem_wait(&game->comms->sem_occupied);
}

/*
 * Esegue la signal del consumatore.
 * @param game  La struttura game_threads.
 */
void signal_consumer(struct game_threads *game)
{
    sem_post(&game->comms->sem_free);
}

/*
 * Esegue la wait del mutex per la scrittura su buffer.
 * @param game  La struttura game_threads.
 */
void wait_mutex(struct game_threads *game)
{
    sem_wait(&game->comms->sem_mutex);
}

/*
 * Esegue la signal del mutex per la scrittura su buffer.
 * @param game  La struttura game_threads.
 */
void signal_mutex(struct game_threads *game)
{
    sem_post(&game->comms->sem_mutex);
}

/*
 * Prende il valore del `sem_occupied`, ovvero degli elementi presenti nel buffer.
 * @param game  La struttura game_threads.
 */
int await_cleanup_count(struct game_threads *game)
{
    int count;
    sem_getvalue(&game->comms->sem_occupied, &count);
    return count;
}

/*
 * Pulisce il buffer di comunicazione.
 * @param game  La struttura game_threads.
 */
void cleanup_comms_buffer(struct game_threads *game)
{
    DEBUG("still awaiting cleanup: %d elements\n", await_cleanup_count(game));
    
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

