#ifndef THREADS_H
#define THREADS_H

#include "../commons/imports.h"
#include "../commons/structures.h"
#include "../commons/drawing.h"
#include "./shared/shared.h"

#define DOUBLE_MAX_CONCURRENCY 48

typedef struct {
    ProductionRules rules;
    void *buffer;
} ThreadCarriage;

typedef struct {
    pthread_t id;
    Component *component;
    ProductionRules rules;
} Thread;


static atomic_int COMMUNICATIONS;
static sem_t POLLING_WRITING;
static sem_t POLLING_READING;
static sem_t SEM_OCCUPIED;
static sem_t SEM_FREE;
static pthread_mutex_t MUTEX;
static pthread_mutex_t BUFFER_MUTEX;


int thread_main(Screen screen, GameSkeleton *game, struct entities_list **entitiesList);

void init_semaphores();
void close_semaphores();

void send_thread_message(int message);
int read_thread_message();

void wait_producer();
void signal_producer();
void wait_consumer();
void signal_consumer();

#endif //THREADS_H
