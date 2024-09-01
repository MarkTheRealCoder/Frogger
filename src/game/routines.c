#include "routines.h"
#include "core.h"
#include "entities.h"
#include "../graphics/drawing.h"
#include "../utils/shortcuts.h"

void *example_routine() 
{
    sleepy(1, TIMEFRAME_SECONDS);
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
            entity->x -= CORE_GAME_FROG_JUMP;
            break;
        case DIRECTION_NORTH:
            entity->y -= CORE_GAME_FROG_JUMP;
            break;
        case DIRECTION_EAST:
            entity->x += CORE_GAME_FROG_JUMP;
            break;
        case DIRECTION_SOUTH:
            entity->y += CORE_GAME_FROG_JUMP;
            break;
        case DIRECTION_STILL:
            break;
    }
}

void handle_packet(struct game_threads *game, Packet *packet)
{
    char *log_message = NULL;

    switch (packet->type)
    {
        case PACKET_TYPE__ENTITYMOVE:
            {
                EntityMovePacket *entity_move_packet = (EntityMovePacket *) packet->data;

                struct entity *packet_entity = &entity_move_packet->entity;
                struct entity *entity = entity_node_find_id(game->entity_node, packet_entity->id);

                Position currentEntityPosition = { 
                    (*entity).x, 
                    (*entity).y
                };

                entity->x += packet_entity->x;
                entity->y += packet_entity->y;
                entity->direction = packet_entity->direction;

                Position newEntityPosition = { 
                    (*entity).x, 
                    (*entity).y
                };
                
                StringArt art = getArt(entity);

                display_entity(COLORCODES_FROG_B, art, currentEntityPosition, newEntityPosition, game->map);

                log_message = concat(5, str_packet_type(packet->type), " at ", 
                                        str_coords(entity), " for ", 
                                        str_entity_type(entity->type));
                break;
            }
        case PACKET_TYPE__TIMER:
            {
                TimerPacket *timer_packet = (TimerPacket *) packet->data;

                if (timer_packet->current_time <= 0)
                {
                    // TODO end manche 
                }
                
                Position timerPosition = { getCenteredX(0) + 25, 2 };

                eraseFor(timerPosition, 1, 40);
                display_clock(timerPosition, timer_packet->current_time, timer_packet->max_time);

                break;
            }
        default:
            break;
    }
    
    Position achievementTitlePosition = { getCenteredX(12) + 72, getCenteredY(25) - 2 };
    Position achievementPosition = { getCenteredX(30) + 75, getCenteredY(25) };

    Position packetLogsTitlePosition = { getCenteredX(20) - 75, getCenteredY(25) - 2 };
    Position packetLogsPosition = { getCenteredX(20) - 75, getCenteredY(25) };

    Position hpsPosition = { getCenteredX(FROG_HPS) - 45, 3 };
    Position scorePosition = { getCenteredX(12), 3 };

    display_string(packetLogsTitlePosition, COLOR_RED, "Packet Logs", 11);
    display_string(achievementTitlePosition, COLOR_RED, "Achievements", 12);

    display_string(scorePosition, COLOR_RED, "Score: XXXXX", 12);

    //addStringToList(&game->achievements->last, COLOR_YELLOW, str_packet_type(packet->type));
    //display_achievements(achievementPosition, 25, 25, *game->achievements);
    
    if (log_message == NULL)
    {
        log_message = str_packet_type(packet->type);
    }

    addStringToList(&game->packet_logs->last, COLOR_YELLOW, log_message);
    display_achievements(packetLogsPosition, 25, 25, *game->packet_logs);

    display_hps(hpsPosition, 0, 5);
}

/*
 * La routine dedicata alla gestione del gioco.
 * @param args  Il pacchetto contenente i dati del gioco.
 */
void *master_routine(void *args)
{
    DEFAULT_ROUTINE_INIT(args)
    DEFAULT_ROUTINE_CONSUMER_INIT

    while (true)
    {
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

/*
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

    Direction dir = -1;

    bool firstIteration = true;

    while (true)
    {
        switch (wgetch(stdscr)) 
        {
            case 'w':
            case 'W':
            case KEY_UP: 
                dir = DIRECTION_NORTH;
                break;
            case 's':
            case 'S':
            case KEY_DOWN: 
                dir = DIRECTION_SOUTH;
                break;
            case 'a':
            case 'A':
            case KEY_LEFT:
                dir = DIRECTION_WEST;
                break;
            case 'd':
            case 'D':
            case KEY_RIGHT: 
                dir = DIRECTION_EAST;
                break;
            case 'p':
            case 'P':
                halt_threads(game);
                break;
            default:
                if (firstIteration)
                {
                    /* 
                     * Movimento in direzione STILL in modo tale da stampare
                     * a schermo la rana nella posizione iniziale.
                     */
                    dir = DIRECTION_STILL;
                    firstIteration = false;
                    break;
                }
                else
                {
                    continue;
                }
        }

        wait_producer(game);

        CHECK_SIGNAL(signal, mutex)

        entity_move_packet.entity.direction = dir;
        move_on_direction(&entity_move_packet);
        
        product = create_packet(&entity_move_packet, 1, PACKET_TYPE__ENTITYMOVE, true);

        /* writing in the communication buffer the created packet. */
        WRITE_TO_COMMS_BUFFER(game, comms_buffer, index, product)
        
        signal_producer(game);

        dir = -1;
    }

    DEBUG("exited from frog\n");

    return NULL;
}

/*
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
