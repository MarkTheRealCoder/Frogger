#define SAFE_VALUE -999


void entity_move(void *_rules) {
    DataBuffer *rules = (DataBuffer*)_rules;
    int value = (int)((int*)rules->samples)[0];
    *(rules->output) = value;
}

void user_listener(void *_rules) {
    DataBuffer *rules = (DataBuffer*)_rules;
    int value = -1;
    while (value == -1) {
        switch(wgetch(stdscr)) {
            case 'W': 
            case 'w':
            case KEY_UP: value = UP;
                break;
            case 'S': 
            case 's': 
            case KEY_DOWN: value = DOWN;
                break; 
            case 'A': 
            case 'a': 
            case KEY_LEFT: value = LEFT;
                break; 
            case 'D': 
            case 'd': 
            case KEY_RIGHT: value = RIGHT;
                break; 
            case 'P': 
            case 'p': value = PAUSE;
                break;
        }
    }
    *(rules->output) = value;
}

void timer_counter(void *_rules) {
    DataBuffer *rules = (DataBuffer*)_rules;
    int value = (int)((int*)rules->samples)[0]; // current value (updated in main routine)
    int part = (int)((int*)rules->samples)[1]; // fraction to be subtracted from value;
    *(rules->output) = value - part;
}

void update_position (struct entity *e, enum Movement movement) {
    e->old_position = e->curr_position;
    if (movement == LEFT || movement == RIGHT) e->curr_position.x += (movement == LEFT) ? -1 : 1;
    else {
        if (e->type == FROG) e->curr_position.y += (movement == TOP) ? -3 : 3;
        else e->curr_position.y += (movement == TOP) ? -1 : 1;
    }
}

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