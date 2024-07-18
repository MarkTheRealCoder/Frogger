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


void *example_routine();
void *master_routine(void *args);
void *example_producer(void *args);
void *example_producer2(void *args);

void *run_timer(void *args);

#endif // !FROGGER_ROUTINES_H

