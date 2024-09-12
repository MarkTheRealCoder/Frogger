#ifndef DRAWING_H
#define DRAWING_H

#include "imports.h"
#include "structures.h"
#include "entities.h"


#define GAME_HIDEOUTS 5
#define GAME_HIDEOUTS_SPACES_INBETWEEN 4

#define GAME_MANCHE_MAXTIME 120 // in secondi
#define GAME_MANCHE_FRACTION 1 // in secondi

#define GAME_ENTITY_SIZE 3

#define GAME_FROG_LIVES 5
#define GAME_FROG_JUMP_X 1
#define GAME_FROG_JUMP_Y GAME_ENTITY_SIZE

#define GAME_LAWN_TOP_LANES 2
#define GAME_LAWN_BOTTOM_LANES 2
#define GAME_RIVER_LANES 8

#define CORE_GAME_MAP_WIDTH                               \
    ((GAME_HIDEOUTS * GAME_ENTITY_SIZE) +                 \
    (GAME_HIDEOUTS_SPACES_INBETWEEN * GAME_ENTITY_SIZE) + \
    (2 * GAME_ENTITY_SIZE))

#define GAME_PROJECTILE_WAIT 2 // in secondi

#define GAME_PLANTS 3
#define GAME_CROCS (2 * GAME_RIVER_LANES)

#define GAME_CROCS_MIN_WIDTH (2 * GAME_ENTITY_SIZE)
#define GAME_CROCS_MAX_WIDTH (3 * GAME_ENTITY_SIZE)

#define MAP_HEIGHT 39

#define WATER_COLOR 51, 51, 255
#define GRASS_COLOR 0, 102, 0
#define BLOCKED_COLOR 22, 247, 236
#define HIDEOUT_COLOR 51, 25, 0
#define SIDEWALK_COLOR 204, 102, 0

#define TOTAL_LIVES 3

#define MAP_START_X 6
#define MAP_START_Y 6
#define MAP_WIDTH 99

#define CLOCK_DISPLAY_TRUE_SIZE 10
#define CLOCK_DISPLAY_SIZE 20

#define GET_MAP_Y(_y, map) (_y) - (map.garden.y - 6)

#define WITHIN_BOUNDARIES(_x, _y, map) (((map).sidewalk.x <= (_x)) && ((_x) < (map).sidewalk.x + (map).width) && ((map).garden.y - FROG_HEIGHT <= (_y)) && ((_y) < (map).sidewalk.y + FROG_HEIGHT))
// ((((map).sidewalk.x) <= (_x)) && ((_x) < (((map).sidewalk.x) + ((map).width)))) && ((((map).garden.y - FROG_HEIGHT) <= (_y)) && ((_y) <= (((map).sidewalk.y))))

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
    TrueType e1;
    TrueType e2;

    int e1_priority;
    int e2_priority;

    enum {
        COLLISION_OVERLAPPING = 0,
        COLLISION_AVOIDED = 1,
        COLLISION_DESTROYING = 3,
        COLLISION_DAMAGING = 48,
        COLLISION_TRANSFORM = 195
    } collision_type;
} CollisionPacket;

enum PS {
    PS_MAIN_MENU,
    PS_PAUSE_MENU,
    PS_LOST,
    PS_WIN,
    PS_VERSION_MENU
};

enum MainMenuOptions {
    MMO_START_NEW,
    MMO_QUIT
};

enum PauseMenuOptions {
    PMO_RESUME,
    PMO_QUIT
};

enum VersionMenuOptions {
    VMO_THREADS,
    VMO_PROCESSES,
    VMO_QUIT
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
        "|__|      /__/     \\__\\  \\______/  |_______/    |_______|"
    };

#define _FROGGER_LOST_LOGO_LENGTH 5
static char *_FROGGER_LOST_LOGO[_FROGGER_LOST_LOGO_LENGTH] =
        {
                "__   __                    _                     _",
                "\\ \\ / /   ___    _   _    | |       ___    ___  | |_",
                " \\ V /   / _ \\  | | | |   | |      / _ \\  / __| | __|",
                "  | |   | (_) | | |_| |   | |___  | (_) | \\__ \\ | |_",
                "  |_|    \\___/   \\__,_|   |_____|  \\___/  |___/  \\__|"
        };

#define _FROGGER_WIN_LOGO_LENGTH 5
static char *_FROGGER_WIN_LOGO[_FROGGER_WIN_LOGO_LENGTH] =
        {
                "__   __                   __        __  _",
                "\\ \\ / /   ___    _   _    \\ \\      / / (_)  _ __",
                " \\ V /   / _ \\  | | | |    \\ \\ /\\ / /  | | | '_ \\",
                "  | |   | (_) | | |_| |     \\ V  V /   | | | | | |",
                "  |_|    \\___/   \\__,_|      \\_/\\_/    |_| |_| |_|"
        };

#define _FROGGER_SCREEN_TOO_SMALL_LENGTH 45
static char _FROGGER_SCREEN_TOO_SMALL[_FROGGER_SCREEN_TOO_SMALL_LENGTH] =
    "Screen size is too small to play Frogger! :(\n";

#define _FROGGER_SCREEN_CORRECT_LENGTH 34
static char _FROGGER_SCREEN_CORRECT_SIZE[_FROGGER_SCREEN_CORRECT_LENGTH] =
        "Minimum size is: 40rows x 100cols\n";

#define HEART "\xE2\x99\xA5"

#define FROG_ART_COLOR 51, 255, 51 // 0 204 0
#define FROG_ART_LOGO_COLOR 255, 255, 0 // 0 0 51
#define FROG_ART_LOGO_COLOR_Q 255, 51, 51 // 153 0 0
#define FROG_ART_SELECTED_COLOR 255, 124, 9 // 255 153 51

WINDOW *init_screen(Screen *screen);
void center_string_colored(char *string, int pair, int max, int cuy);
unsigned int show(Screen screen, enum PS prog_state, int *output);
void display_clock(Position p, int value, unsigned int max);
void display_hps(Position position, int lives);
void addStringToList(StringNode **list, int color, char *string);

void display_string(Position position, int color, const char *string, int length);
void display_achievements(Position p, short length, short height, StringList list);
void delete_entity_pos(const int height, const int length, const Position last, const MapSkeleton map);
void display_entity(int fg, StringArt art, Position curr, Position last, MapSkeleton map);

void make_MapSkeleton(MapSkeleton *map, Position sp, int width);
MapSkeleton display_map(Position sp, int width, MapSkeleton map);

void center_string(char str[], int max, int cuy);
void eraseFor(Position sp, int height, int length);

void handle_screen_resize();

enum color_codes getEntityColor(enum entity_type type);

Position getPosition(int x, int y);

Position getPositionFromEntity(Entity e);
int getPriorityByEntityType(EntityType entityType);
int getHeightByEntityType(EntityType entityType);

Cuboid getCuboidFromEntity(Entity e);
CollisionPacket areColliding(Entity e1, Entity e2);

void draw(struct entities_list *es, MapSkeleton *map, Clock *timers, StringList *achievements, int score, int lives, bool drawAll);
void display_game_over(Screen screen, int result);

void display_debug_string(int y, const char *__restrict __format, int stringLength, ...);

#endif
