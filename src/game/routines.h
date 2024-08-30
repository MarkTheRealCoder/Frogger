#ifndef FROGGER_ROUTINES_H
#define FROGGER_ROUTINES_H

#include "core.h"


// Se il segnale è GAMESIGNAL_STOP, esce dal ciclo di esecuzione.
#define IF_SIGNAL_STOP(signal)                  \
    if (atomic_load(signal) == GAMESIGNAL_STOP) \
    {                                           \
        break;                                  \
    }

// Se il segnale è GAMESIGNAL_HALT, blocca il mutex.
#define IF_SIGNAL_HALT(signal, mutex)           \
    if (atomic_load(signal) == GAMESIGNAL_HALT) \
    {                                           \
        pthread_mutex_lock(mutex);              \
        pthread_mutex_unlock(mutex);            \
    }

// Controlla se il segnale è GAMESIGNAL_HALT o GAMESIGNAL_STOP.
#define CHECK_SIGNAL(signal, mutex) \
    IF_SIGNAL_HALT(signal, mutex)   \
    IF_SIGNAL_STOP(signal)

/*
 * Inizializzazione di ogni routine.
 *
 * Le seguenti variabili saranno esposte dopo l'evocazione di questa macro:
 *
 *  - `Packet *packet` - Il pacchetto inviato alla routine.
 *  - `struct game_threads *game` - I thread del gioco. Contiene i segnali e i mutex.
 *  
 *  - `atomic_int *signal` - Il segnale di esecuzione.
 *  - `pthread_mutex_t *mutex` - Il mutex di esecuzione.
 *
 *  - `Packet **comms_buffer` - Il buffer di comunicazione.
 *  - `int buffer_size` - La dimensione del buffer di comunicazione.
 */
#define DEFAULT_ROUTINE_INIT(args)                                      \
    Packet *packet = (Packet *) args;                                   \
    struct game_threads *game = (struct game_threads *) packet->data;   \
                                                                        \
    atomic_int *signal = &game->time.signal;                            \
    pthread_mutex_t *mutex = &game->time.mutex;                         \
                                                                        \
    Packet **comms_buffer = (Packet **) game->comms->buffer;            \
    int buffer_size = game->comms->buffer_size;

/*
 * Termina l'inizializzazione di ogni routine consumatore.
 *
 * Le seguenti variabili saranno esposte dopo l'evocazione di questa macro:
 *
 * - `Packet *consumed_product` - Il prodotto da consumare dal buffer di comunicazione.
 * - `int index` - L'indice del buffer di comunicazione.
 */
#define DEFAULT_ROUTINE_CONSUMER_INIT           \
    Packet *consumed_product;                   \
    int index = 0;

/*
 * Termina l'inizializzazione di ogni routine produttore.
 *
 * Le seguenti variabili saranno esposte dopo l'evocazione di questa macro:
 *
 * - `Packet *consumed_product` - Il prodotto da inviare al buffer di comunicazione.
 * - `int index` - L'indice del buffer di comunicazione.
 */
#define DEFAULT_ROUTINE_PRODUCER_INIT           \
    Packet *product;                            \
    int *index = &game->comms->next_prod_index;

/*
 * Legge un prodotto dal buffer di comunicazione.
 * Esegue il blocco del mutex prima di leggere e lo sblocca dopo aver letto
 */
#define READ_FROM_COMMS_BUFFER(game, comms_buffer, index, consumed_product) \
        wait_mutex(game);                                                   \
        consumed_product = comms_buffer[index];                             \
                                                                            \
                                                                            \
        index = (index + 1) % buffer_size;                                  \
        signal_mutex(game);

/*
        DEBUG("Read\t\t: index: %d\ttype: %s\n",                            \
                index, str_packet_type(consumed_product->type))             \*/

/*
 * Scrive un prodotto nel buffer di comunicazione.
 * Esegue il blocco del mutex prima di scrivere e lo sblocca dopo aver scritto.
 */
#define WRITE_TO_COMMS_BUFFER(game, comms_buffer, index, product)   \
    wait_mutex(game);                                               \
    comms_buffer[*index] = product;                                 \
                                                                    \
                                                                    \
    *index = (*index + 1) % buffer_size;                            \
    signal_mutex(game);

/*
    DEBUG("Produced\t: index: %d\ttype: %s\n",                      \
            *index, str_packet_type(product->type));                \
            */

void *example_routine();
void *master_routine(void *args);

void *timer_routine(void *args);
void *frog_projectile_routine(void *args);
void *frog_routine(void *args);

#endif // !FROGGER_ROUTINES_H

