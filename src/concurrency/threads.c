#include "threads.h"

PollingResult thread_polling_routine(int buffer[MAX_CONCURRENCY], GameSkeleton *game)
{
    PollingResult pollingResult = POLLING_NONE;

    sem_wait(&POLLING_WRITING);
    sem_wait(&POLLING_READING);

    for (int i = 0; i < MAX_CONCURRENCY; i++)
    {
        int value = buffer[i];

        if (value == COMMS_EMPTY)
        {
            continue;
        }

        buffer[i] = COMMS_EMPTY;
        Component *c = find_component(i, game);
        
        switch(c->type)
        {
            case COMPONENT_CLOCK:
                pollingResult = handle_clock(c, value);
                break;
            case COMPONENT_ENTITY:
                pollingResult = handle_entity(c, value, true);
                break;
            case COMPONENT_ENTITIES:
                pollingResult = handle_entities(c, value);
                break;
        }
    }

    sem_post(&POLLING_READING);
    sem_post(&POLLING_WRITING);

    return pollingResult;
}

void generic_thread(void *packet)
{
    Packet *p = (Packet*) packet;
    void (*producer)(void*) = p->producer;
    unsigned int id = p->id;
    unsigned int index = 0;
    while ((id >> index) != 1) index++;

    ThreadCarriage *carriage = (ThreadCarriage*) p->carriage;
    int *buffer = (int*) carriage->buffer;
    ProductionRules rules = carriage->rules;

    SystemMessage action = MESSAGE_NONE;
    while (true) {

        if (MATCH_ID(id, COMMUNICATIONS)) {
            SystemMessage msg = COMMUNICATIONS & MESSAGE_RUN;
            action = (msg != action && msg != MESSAGE_NONE) ? msg : action;
        }

        if (action == MESSAGE_RUN) {

            sem_wait(&POLLING_READING);
            sem_post(&POLLING_READING);

            int sem_val = 0;
            sem_getvalue(&POLLING_WRITING, &sem_val);
            if (sem_val) {

                producer(&rules);
                sem_wait(&POLLING_WRITING);

                if (buffer[index] == COMMS_EMPTY) buffer[index] = rules.buffer;

                sem_post(&POLLING_WRITING);
                sleep(1);
            }
        }
        else if (action == MESSAGE_STOP) {
            return;
        }
    }
}

void thread_main(GameSkeleton *game, struct entities_list **list)
{
    init_semaphores();
    int buffer[MAX_CONCURRENCY] = {COMMS_EMPTY};

    while (true)
    {
        PollingResult result = thread_polling_routine(buffer, game);

        switch (result)
        {
            case POLLING_FROG_DEAD:     // Manche lost
                break;
            case POLLING_GAME_PAUSE:    // Pause
                break;
            case POLLING_MANCHE_LOST:   // Manche lost
                break;
        }

        // Validazione delle entità
        // Creazione di nuove entità
        // Collisioni
        // Rimozione entità e aggiornamento
        // Display

        sleepy(100, TIMEFRAME_MILLIS);
    }

    close_semaphores();
}

/**
 * Inizializza i semafori.
 */
void init_semaphores()
{
    sem_init(&COMMUNICATION_SEMAPHORE, 0, 1);
    sem_init(&POLLING_WRITING, 0, 1);
    sem_init(&POLLING_READING, 0, 1);
}

/**
 * Distrugge i semafori.
 */
void close_semaphores()
{
    sem_close(&COMMUNICATION_SEMAPHORE);
    sem_close(&POLLING_WRITING);
    sem_close(&POLLING_READING);
}