#ifndef DRAWING_H
#define DRAWING_H

#include "imports.h"
#include "structures.h"
#include "entities.h"

#define CORE_GAME_HIDEOUTS 5
#define CORE_GAME_HIDEOUTS_SPACES_INBETWEEN 4
#define CORE_GAME_MANCHE_MAXTIME 120 // in secondi
#define CORE_GAME_MANCHE_FRACTION 1 // in secondi

#define CORE_GAME_ENTITY_SIZE 3

#define CORE_GAME_FROG_LIVES 5
#define CORE_GAME_FROG_JUMP_X 1
#define CORE_GAME_FROG_JUMP_Y CORE_GAME_ENTITY_SIZE

#define CORE_GAME_LAWN_TOP_LANES 2
#define CORE_GAME_LAWN_BOTTOM_LANES 2
#define CORE_GAME_RIVER_LANES 8

#define CORE_GAME_MAP_WIDTH                                         \
    ((CORE_GAME_HIDEOUTS * CORE_GAME_ENTITY_SIZE) +                 \
    (CORE_GAME_HIDEOUTS_SPACES_INBETWEEN * CORE_GAME_ENTITY_SIZE) + \
    (2 * CORE_GAME_ENTITY_SIZE))

#define CORE_GAME_PROJECTILE_WAIT 2 // in secondi

#define CORE_GAME_PLANTS 3
#define CORE_GAME_CROCS (2 * CORE_GAME_RIVER_LANES)

#define CORE_GAME_CROCS_MIN_WIDTH (2 * CORE_GAME_ENTITY_SIZE)
#define CORE_GAME_CROCS_MAX_WIDTH (3 * CORE_GAME_ENTITY_SIZE)

static bool SCREEN_INVALID_SIZE = false;

#define MAP_HEIGHT 39

#define WATER_COLOR 51, 51, 255
#define GRASS_COLOR 0, 102, 0
#define BLOCKED_COLOR 22, 247, 236
#define HIDEOUT_COLOR 51, 25, 0
#define SIDEWALK_COLOR 204, 102, 0

#define TOTAL_LIVES 3
#define MAP_WIDTH 100

#define PERCENTAGE_MAP_X 50
#define PERCENTAGE_MAP_Y 80

#define PERCENTAGE_TIMER_X 10
#define PERCENTAGE_TIMER_Y 5

#define PERCENTAGE_TTIMER_X 10
#define PERCENTAGE_TTIMER_Y 5

#define PERCENTAGE_HPS_X 10
#define PERCENTAGE_HPS_Y 5

#define PERCENTAGE_ACHI_X 20
#define PERCENTAGE_ACHI_Y 50


#define JOLLY_PAIR 127

#define GET_MAP_Y(_y, map) (_y) - (map.hideouts[0].y - 3)

#define WITHIN_BOUNDARIES(_x, _y, map) ((map.sidewalk.x) <= (_x) && ((_x) < ((map.sidewalk.x) + (map.width)))) && ((map.hideouts[0].y) <= (_y) && ((_y) < ((map.sidewalk.y) + (3))))

/* COLOR CODES */

enum color_codes {
    COLORCODES_CROC_B = 10,
    COLORCODES_CROC_A,
    COLORCODES_FROG_B,
    COLORCODES_FROG_A,
    COLORCODES_PLANT_B,
    COLORCODES_PLANT_A,
    COLORCODES_PROJECTILE_F,
    COLORCODES_PROJECTILE_FL,
    COLORCODES_WATER,
    COLORCODES_GRASS,
    COLORCODES_HIDEOUT,
    COLORCODES_SIDEWALK,
    COLORCODES_BLOCKED,
    COLORCODES_FROG_ART,
    COLORCODES_FROG_ART_LOGO,
    COLORCODES_FROG_ART_LOGO_QUIT,
    COLORCODES_FROG_ART_SELECTED
};

typedef struct {
    Position leftcorner;
    Position rightcorner;
} Cuboid;

typedef struct {
    EntityType e1;
    EntityType e2;

    int e1_priority;
    int e2_priority;

    enum {
        COLLISION_OVERLAPPING, 
        COLLISION_DAMAGING,
        COLLISION_AVOIDED
    } collision_type;
} CollisionPacket;

typedef struct {
    unsigned int x;
    unsigned int y;
} Screen;

enum PS {
    PS_MAIN_MENU,
    PS_PAUSE_MENU,
    PS_SAVINGS,
    PS_CREATE_SAVING,
    PS_LOST,
    PS_WIN
};

enum MainMenuOptions {
    MMO_START_NEW,
    MMO_OPEN_SAVING,
    MMO_CREATE_SAVING,
    MMO_QUIT
};

enum PauseMenuOptions {
    PMO_RESUME,
    PMO_SAVE_CURRENT,
    PMO_SAVE_REPLAY,
    PMO_CREATE_SAVING,
    PMO_SAVE_AND_QUIT,
    PMO_QUIT
};


/* FROG ARTS */
#define _FROG_ART_LENGTH 20
static char *_FROG_ART[_FROG_ART_LENGTH] =
    {
        "\n",
        "____  __.---\"\"---.__  ____\n",
        "/####\\/              \\/####\\\n",
        "(   /\\ )              ( /\\   )\n",
        "\\____/                \\____/\n",
        "__/                          \\__\n",
        ".-\"    .                      .    \"-.\n",
        "|  |   \\.._                _../   |  |\n",
        " \\  \\    \\.\"-.__________.-\"./    /  /\n",
        "\\  \\    \"--.________.--\"    /  /\n",
        "___\\  \\_                    _/  /___\n",
        "./    )))))                  (((((    \\.\n",
        "\\                                      /\n",
        "\\           \\_          _/           /\n",
        "\\    \\____/\"\"-.____.-\"\"\\____/    /\n",
        "\\    \\                  /    /\n",
        "\\.  .|                |.  ./\n",
        " .\" / |  \\              /  | \\  \".\n",
        ".\"  /   |   \\            /   |   \\   \".\n",
        "/.-./.--.|.--.\\          /.--.|.--.\\.-.|\"\n"
    };

#define _FROGGER_LOGO_LENGTH 6
static char *_FROGGER_LOGO[_FROGGER_LOGO_LENGTH] =
    {
        "    ______                                \n",
        "   / ____/________  ____ _____ ____  _____\n",
        "  / /_  / ___/ __ \\/ __ `/ __ `/ _ \\/ ___/\n",
        " / __/ / /  / /_/ / /_/ / /_/ /  __/ /    \n",
        "/_/   /_/   \\____/\\__, /\\__, /\\___/_/     \n",
        "                 /____//____/             \n\n"
    };

#define _FROGGER_PAUSE_LENGTH 8
static char *_FROGGER_PAUSE_ART[_FROGGER_PAUSE_LENGTH] = {
    "              _         _              ",
    "  __   ___.--'_`.     .'_`--.___   __  ",
    " ( _`.'. -   'o` )   ( 'o`   - .`.'_ ) ",
    " _\\.'_'      _.-'     `-._      `_`./_ ",
    "( \\`. )    //\\`         '/\\    ( .'/ )",
    " \\_`-'`---'\\__,       ,__//`---'`-'_/ ",
    "  \\`        `-\\         /-'        '/  ",
    "   `                               '   "
};

#define _FROGGER_PAUSE_LOGO_LENGTH 6
static char *_FROGGER_PAUSE_LOGO[_FROGGER_PAUSE_LOGO_LENGTH] =
    {
        ".______        ___       __    __       _______. _______ ",
        "|   _  \\      /   \\     |  |  |  |     /       ||   ____|",
        "|  |_)  |    /  ^  \\    |  |  |  |    |   (----`|  |__   ",
        "|   ___/    /  /_\\  \\   |  |  |  |     \\   \\    |   __|  ",
        "|  |       /  _____  \\  |  `--'  | .----)   |   |  |____ ",
        "| _|      /__/     \\__\\  \\______/  |_______/    |_______|"
    };

#define HEART "\xE2\x99\xA5"

#define FROG_ART_COLOR 51, 255, 51 // 0 204 0
#define FROG_ART_LOGO_COLOR 255, 255, 0 // 0 0 51
#define FROG_ART_LOGO_COLOR_Q 255, 51, 51 // 153 0 0
#define FROG_ART_SELECTED_COLOR 255, 124, 9 // 255 153 51

void init_screen(Screen *scrn);
void center_string_colored(char *string, int pair, int max, int cuy);
unsigned int show(Screen scr, enum PS prog_state, int *output);
void display_clock(Position p, short value, short max);
void display_hps(Position position, short lives);
void addStringToList(StringNode **list, int color, char *string);

void display_string(const Position position, const int color, const char *string, int length);
void display_achievements(Position p, short length, short height, StringList list);
void display_entity(int fg, StringArt art, Position curr, Position last, MapSkeleton map);
MapSkeleton display_map(Position sp, int width, MapSkeleton* map);

void center_string(char str[], int max, int cuy);
void eraseFor(Position sp, short height, short length);

void handle_screen_resize();

enum color_codes getEntityColor(enum entity_type type);

void setScreenValidity(bool value);
bool isScreenValid();

Position getPosition(int x, int y);

Entity entities_default_frog();
Entity entities_default_plant(int index);
Entity entities_default_croc(int index);

Component *find_component(int index, GameSkeleton *game);
void update_position (Entity *e, Action movement);

Position getPositionFromEntity(Entity e);
int getPriorityByEntityType(EntityType entityType);
Cuboid getCuboidFromEntity(Entity e);
CollisionPacket areColliding(Entity e1, Entity e2);


void draw(struct entities_list *es, MapSkeleton *map, Clock *timers, StringList *achievements, int score, int lives, bool drawAll);

#endif
