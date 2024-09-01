#include "routines.h"
#include "core.h"
#include "entities.h"
#include "../graphics/drawing.h"
#include "../utils/shortcuts.h"
#include "../utils/globals.h"

void *example_routine() 
{
    sleepy(10000, TIMEFRAME_SECONDS);
    return NULL; 
}

void move_on_direction(EntityMovePacket *entity_move_packet)
{
    struct entity *entity = &entity_move_packet->entity;

    entity->x = 0;
    entity->y = 0;

    switch (entity->direction)
    {
        case DIRECTION_WEST:
            entity->x -= CORE_GAME_FROG_JUMP_X;
            break;
        case DIRECTION_NORTH:
            entity->y -= CORE_GAME_FROG_JUMP_Y;
            break;
        case DIRECTION_EAST:
            entity->x += CORE_GAME_FROG_JUMP_X;
            break;
        case DIRECTION_SOUTH:
            entity->y += CORE_GAME_FROG_JUMP_Y;
            break;
        case DIRECTION_STILL:
            break;
    }
}

bool frog_wgetch(Direction *direction, bool *firstIteration)
{
    switch (wgetch(stdscr))
    {
        case 'w':
        case 'W':
        case KEY_UP:
            *direction = DIRECTION_NORTH;
            break;
        case 's':
        case 'S':
        case KEY_DOWN:
            *direction = DIRECTION_SOUTH;
            break;
        case 'a':
        case 'A':
        case KEY_LEFT:
            *direction = DIRECTION_WEST;
            break;
        case 'd':
        case 'D':
        case KEY_RIGHT:
            *direction = DIRECTION_EAST;
            break;
        case 'p':
        case 'P':
            setGamePaused(true);
            break;
        case 'f':
        case 'F':
            // todo fire projectile
        default:
            if (*firstIteration)
            {
                /*
                 * Movimento in direzione STILL in modo tale da stampare
                 * a schermo la rana nella posizione iniziale.
                 */
                *firstIteration = false;
                break;
            }
            return true;
    }

    return false;
}

void handle_packet_entityMove(struct game_threads *game, Packet *packet)
{
    EntityMovePacket *entity_move_packet = (EntityMovePacket *) packet->data;

    struct entity *packet_entity = &entity_move_packet->entity;
    struct entity *entity = entity_node_find_id(game->entity_node, packet_entity->id);

    Position currentEntityPosition = getPositionFromEntity(*entity);

    entity->x += packet_entity->x;
    entity->y += packet_entity->y;
    entity->direction = packet_entity->direction;

    Position newEntityPosition = getPositionFromEntity(*entity);

    bool isFrog = entity->type == ENTITY_TYPE__FROG;

    if (isFrog)
    {
        // achievement "So it begins..."

        if (entity->y <= 25)
        {
            // achievement "Halfway there"
        }
    }

    StringArt art = getArt(entity);
    enum color_codes color = getEntityColor(entity->trueType);

    display_entity(color, art, currentEntityPosition, newEntityPosition, game->map);
}

void handle_packet_timer(struct game_threads *game, Packet *packet)
{
    TimerPacket *timer_packet = (TimerPacket *) packet->data;

    if (timer_packet->current_time <= 0)
    {
        // TODO end game
    }

    Position timerPosition = { getCenteredX(0) + 25, 2 };

    eraseFor(timerPosition, 1, 40);
    display_clock(timerPosition, timer_packet->current_time, timer_packet->max_time);
}

void handle_packet(struct game_threads *game, Packet *packet)
{
    switch (packet->type)
    {
        case PACKET_TYPE__ENTITYMOVE:
            handle_packet_entityMove(game, packet);
            break;
        case PACKET_TYPE__TIMER:
            handle_packet_timer(game, packet);
            break;
        default:
            break;
    }

    Position achievementTitlePosition = { getCenteredX(12) + 72, getCenteredY(25) - 2 };
    Position achievementPosition = { getCenteredX(30) + 75, getCenteredY(25) };

    Position hpsPosition = { getCenteredX(FROG_HPS) - 45, 3 };
    Position scorePosition = { getCenteredX(12), 3 };

    display_string(achievementTitlePosition, COLOR_RED, "Achievements", 12);

    display_string(scorePosition, COLOR_RED, "Score: XXXXX", 12);

    //addStringToList(&game->achievements->last, COLOR_YELLOW, str_packet_type(packet->type));
    //display_achievements(achievementPosition, 25, 25, *game->achievements);

    display_hps(hpsPosition, 0, 5);
}

/**
 * La routine dedicata alla gestione del gioco.
 * @param args  Il pacchetto contenente i dati del gioco.
 */
void *master_routine(void *args)
{
    DEFAULT_ROUTINE_INIT(args)
    DEFAULT_ROUTINE_CONSUMER_INIT

    while (true)
    {
        if (isGamePaused())
        {
            halt_threads(game);
        }

        wait_consumer(game);
        
        CHECK_SIGNAL(signal, mutex)

        /* consumption of the packet from the communication buffer. */
        READ_FROM_COMMS_BUFFER(game, comms_buffer, index, consumed_product)

        handle_packet(game, consumed_product);
        
        /* destruction of the now-consumed packet. */
        destroy_packet(consumed_product);

        signal_consumer(game);
    }

    DEBUG("exited from master\n");

    return NULL;
}

/**
 * La routine dedicata alla modifica e comunicazione del tempo di gioco.
 * @param args  Il pacchetto contenente i dati del gioco.
 */
void *timer_routine(void *args) 
{
    DEFAULT_ROUTINE_INIT(args)
    DEFAULT_ROUTINE_PRODUCER_INIT

    TimerPacket timer_packet = { };
    timer_packet.current_time = CORE_GAME_MANCHE_MAXTIME;
    timer_packet.max_time = CORE_GAME_MANCHE_MAXTIME;

    while (true)
    {
        wait_producer(game);

        CHECK_SIGNAL(signal, mutex)
     
        /* generation of the packet with its contents */
        timer_packet.current_time -= CORE_GAME_MANCHE_FRACTION;
        product = create_packet(&timer_packet, 1, PACKET_TYPE__TIMER, false);
        
        /* writing in the communication buffer the created packet. */
        WRITE_TO_COMMS_BUFFER(game, comms_buffer, index, product)

        signal_producer(game);

        sleepy(CORE_GAME_MANCHE_FRACTION, TIMEFRAME_SECONDS);
    }

    DEBUG("exited from timer\n");

    return NULL;
}

void *frog_routine(void *args) 
{
    DEFAULT_ROUTINE_INIT(args)
    DEFAULT_ROUTINE_PRODUCER_INIT
    
    struct entity *frog = entity_node_find_id(game->entity_node, ENTITIES_FROG_ID);

    EntityMovePacket entity_move_packet = { };
    entity_move_packet.entity = *frog; // local clone

    Direction direction = DIRECTION_STILL;
    bool firstIteration = true;

    while (true)
    {
        if (frog_wgetch(&direction, &firstIteration))
        {
            continue;
        }

        wait_producer(game);

        CHECK_SIGNAL(signal, mutex)

        entity_move_packet.entity.direction = direction;
        move_on_direction(&entity_move_packet);
        
        product = create_packet(&entity_move_packet, 1, PACKET_TYPE__ENTITYMOVE, true);

        /* writing in the communication buffer the created packet. */
        WRITE_TO_COMMS_BUFFER(game, comms_buffer, index, product)
        
        signal_producer(game);

        direction = -1;
    }

    DEBUG("exited from frog\n");

    return NULL;
}

/**
 * La routine dedicata alla gestione dei proiettili della rana.
 * @param args  Il pacchetto contenente i dati del gioco.
 */
void *frog_projectile_routine(void *args) 
{
    DEFAULT_ROUTINE_INIT(args)
    DEFAULT_ROUTINE_PRODUCER_INIT

    while (true)
    {
        wait_producer(game);

        CHECK_SIGNAL(signal, mutex)
        
        int test = 0;
        /* generation of the packet with its contents */
        product = create_packet(&test, 1, PACKET_TYPE__INT, true);
        
        /* writing in the communication buffer the created packet. */
        WRITE_TO_COMMS_BUFFER(game, comms_buffer, index, product)
        
        signal_producer(game);

        sleepy(CORE_GAME_MANCHE_FRACTION, TIMEFRAME_SECONDS);
    }

    DEBUG("exited from frog projectile\n");

    return NULL;
}

/**
 * La routine dedicata alla gestione dei coccodrilli.
 * @param args  Il pacchetto contenente i dati del gioco.
 */ 
void *crocodile_routine(void *args)
{
    DEFAULT_ROUTINE_INIT(args)
    DEFAULT_ROUTINE_PRODUCER_INIT

    static int crocId = 0; // todo keep an eye on this
    crocId++;

    struct entity *croc = entity_node_find_id(game->entity_node, crocId);
    
    EntityMovePacket entity_move_packet = { };
    entity_move_packet.entity = *croc; // local clone
    
    while (true)
    {
        wait_producer(game);

        CHECK_SIGNAL(signal, mutex)
        
        move_on_direction(&entity_move_packet);
     
        /* generation of the packet with its contents */
        product = create_packet(&entity_move_packet, 1, PACKET_TYPE__ENTITYMOVE, true);
        
        /* writing in the communication buffer the created packet. */
        WRITE_TO_COMMS_BUFFER(game, comms_buffer, index, product)
        
        signal_producer(game);

        sleepy(1, TIMEFRAME_SECONDS);
    }

    DEBUG("exited from crocodile\n");

    return NULL;
}
