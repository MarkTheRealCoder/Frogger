#ifndef DRAWING_H
#define DRAWING_H
#include <ncurses.h>
#include "entities.h"
#include "map.h"
#include "../utils/shortcuts.h"

#define JOLLY_PAIR 127

/* COLOR CODES */

enum color_codes {
    COLORCODES_CROC_B = 10,
    COLORCODES_CROC_A,
    COLORCODES_FROG_B,
    COLORCODES_FROG_A,
    COLORCODES_FLOWER_B,
    COLORCODES_FLOWER_A,
    COLORCODES_PROJECTILE_F,
    COLORCODES_PROJECTILE_FL,
    COLORCODES_WATER,
    COLORCODES_GRASS,
    COLORCODES_HIDEOUT,
    COLORCODES_SIDEWALK,
    COLORCODES_FROG_ART,
    COLORCODES_FROG_ART_LOGO,
    COLORCODES_FROG_ART_LOGO_QUIT,
    COLORCODES_FROG_ART_SELECTED
};

typedef struct {
    int x;
    unsigned int y;
} Position;

typedef struct {
    enum entity_type type;
    Position pos;       // UP-LEFT CORNER
} Drawing;

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
    PS_GAME
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

#define _MENU_CHOICES_LENGTH 4
static char *_MENU_CHOICES[_MENU_CHOICES_LENGTH] =
    {
        "Start new game\n",
        "Open a saving\n",
        "Create a new saving\n",
        "Quit\n"
    };

#define FROG_ART_COLOR 0, 204, 0
#define FROG_ART_LOGO_COLOR 255, 255, 51
#define FROG_ART_LOGO_COLOR_Q 153, 0, 0
#define FROG_ART_SELECTED_COLOR 255, 153, 51

void init_screen(Screen *scrn);
unsigned int show(const Screen scr, const enum PS prog_state);

#endif





