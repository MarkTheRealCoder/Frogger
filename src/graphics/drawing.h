#ifndef DRAWING_H
#define DRAWING_H

#include "entities.h"
#include "map.h"

static bool SCREEN_INVALID_SIZE = false;

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
    unsigned int length;
    char **art;
} StringArt;

typedef struct _string_node{
    struct _string_node *next;
    struct _string_node *prev;
    char *string;
    int color;
    int length;
} StringNode;

typedef struct {
    StringNode *last;
    int nodes;
} StringList;

typedef struct {
    int x_from;
    int x_to;
    int y_from;
    int y_to;
} Range;

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    enum entity_type type;
    Position pos;       // UP-LEFT CORNER
} Drawing;

typedef struct {
    Position *hideouts;
    Position sidewalk;
    Position river;
    Position garden;
    int width;
} MapSkeleton;

typedef struct {
    unsigned int x;
    unsigned int y;
} Screen;

typedef struct {
    char *top;
    char *middle;
    char *bottom;
} EntityObject;

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

void draw(struct entity_node *es, MapSkeleton *map, Clock *timers, StringList *achievements, int score, int lives, bool drawAll);

#endif
