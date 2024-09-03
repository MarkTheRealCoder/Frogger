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
                pollingResult = handle_clock(c->component, value);
                break;
            case COMPONENT_ENTITY:
                pollingResult = handle_entity(c->component, value, true);
                break;
            case COMPONENT_ENTITIES:
                pollingResult = handle_entities(c->component, value);
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
            action  = COMMUNICATIONS & MESSAGE_RUN;
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


void thread_main(int argc, char **argv) {
    GameSkeleton game = {.current_plants = 0, .current_projectiles = 0, .current_frog_projectiles = 0};
    setup_map(&game);
    // todo to be continued
}