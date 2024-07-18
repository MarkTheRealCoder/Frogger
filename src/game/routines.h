#ifndef FROGGER_ROUTINES_H
#define FROGGER_ROUTINES_H

#include "core.h"

#define IF_SIGNAL_STOP(signal)                  \
    if (atomic_load(signal) == GAMESIGNAL_STOP) \
    {                                           \
        break;                                  \
    }

#define IF_SIGNAL_HALT(signal, mutex)           \
    if (atomic_load(signal) == GAMESIGNAL_HALT) \
    {                                           \
        pthread_mutex_lock(mutex);              \
        pthread_mutex_unlock(mutex);            \
    }

#define CHECK_SIGNAL(signal, mutex) \
    IF_SIGNAL_HALT(signal, mutex)   \
    IF_SIGNAL_STOP(signal)

void *example_routine();
void *master_routine(void *args);
void *example_producer(void *args);
void *example_producer2(void *args);


void *run_timer(void *args);


#endif // !FROGGER_ROUTINES_H

