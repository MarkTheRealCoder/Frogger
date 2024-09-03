#define SAFE_VALUE -999


int thread_polling_routine(int buffer[MAX_THREADS], struct game_data game) {
    PollingSignal signal = 0;
    // sem_wait writing
    // sem_wait reading
    for (int i = 0; i < MAX_THREADS; i++) {
        int value = buffer[i];
        if (value == SAFE_VALUE) continue;
        else buffer[i] = SAFE_VALUE;
        Component *c = find_component(i, game);
        switch(c->type) {
            case CLOCK: {
                struct clock *cl = (struct clock*)c->payload;
                cl->value = value;
                //pthread_mutex / sem_wait
                cl->current = value;
                //pthread_mutex / sem_wait
                if (cl->type == MAIN_CLOCK && cl->value <= 0) signal = MANCHE_LOST;
                else if (cl->type == TEMPORARY_CLOCK && cl->value <= 0) signal = FROG_DEAD; //This must be checked by collision checker
            } break;
            case ENTITY: {
                struct entity *e = (struct entity*)c->payload;
                if (UP <= value && value <= LEFT) update_position(e, value);
                else signal = value; // RQPAUSE
            } break;
        }
    }
    // sem_post reading
    // sem_post writing
    return signal;
}