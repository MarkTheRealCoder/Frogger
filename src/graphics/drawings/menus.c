#include "../drawing.h"

void print_art(StringArt art, int *cuy, int max) {
    int pair = alloc_pair(COLORCODES_FROG_ART, COLOR_BLACK);

    attron(COLOR_PAIR(pair) | A_BOLD);
    for (size_t i = 0; i < art.length; i++)
    {
        center_string(art.art[i], max, *cuy);
        (*cuy)++;
    }
    attroff(COLOR_PAIR(pair) | A_BOLD);
}

/**
 * Stampa il logo del gioco.
 * @param color Il colore da utilizzare.
 * @param max La larghezza della finestra
 */
void print_logo(StringArt logo, int pair, int max, int *cuy)
{
    attron(COLOR_PAIR(pair) | A_BOLD);
    for (size_t i = 0; i < logo.length; i++)
    {
        center_string(logo.art[i], max, *cuy);
        (*cuy)++;
    }
    attroff(COLOR_PAIR(pair) | A_BOLD);
    refresh();
}

/**
 * Stampa le scelte del menu'.
 * @param current_choice L'indice corrente del menu'.
 * @param max La larghezza della finestra.
 */
void print_choices(char **choices, int choices_num, int current_choice, int max, int *cuy)
{
    int standard    = alloc_pair(COLORCODES_FROG_ART_LOGO, COLOR_BLACK);
    int quit        = alloc_pair(COLORCODES_FROG_ART_LOGO_QUIT, COLOR_BLACK);
    int selected    = alloc_pair(COLORCODES_FROG_ART_SELECTED, COLOR_BLACK);
    int starting_choice = current_choice <= 0 ? 0 : current_choice > choices_num - 3 ? choices_num - 3 : current_choice - 1;
    for (size_t i = starting_choice; i <= starting_choice + 2; i++)
    {
        (*cuy)++;
        if (current_choice == i)
        {
            char *tmp = concat(3, "> ", choices[i], " <\n");

            center_string_colored(tmp, selected, max, *cuy-1);
            free(tmp);
            continue;
        }
        center_string_colored(choices[i], i == (choices_num - 1) ? quit : standard, max, *cuy-1);
    }
    refresh();
}

bool menu_listener(int *choice, int choices_num)
{
    int c = wgetch(stdscr);
    bool not_exit = true;

    switch (c)
    {
        case KEY_DOWN:
        case 's':
            if (*choice < choices_num - 1)
            {
                (*choice)++;
            }
            break;
        case KEY_UP:
        case 'w':
            if (*choice)
            {
                (*choice)--;
            }
            break;
        case KEY_ENTER:
        case '\r':
        case '\n':
            not_exit = false;
            break;
    }

    return not_exit;
}

int generic_menu(const Screen scr, StringArt choices, StringArt logo, StringArt art) 
{
    erase();

    int standard = alloc_pair(COLORCODES_FROG_ART_LOGO, COLOR_BLACK);
    int quit = alloc_pair(COLORCODES_FROG_ART_LOGO_QUIT, COLOR_BLACK);
    int choice = 0;
    int cuy = 1;
    int last_logo_pair = standard;
    print_art(art, &cuy, scr.x);
    cuy++;
    int logo_cuy = cuy;

    do 
    {
        cuy = logo_cuy;
        print_logo(logo, choice == choices.length - 1 ? quit : standard, scr.x, &cuy);
        cuy += 4;
        print_choices(choices.art, choices.length, choice, scr.x, &cuy);
    } while(menu_listener(&choice, choices.length));

    return choice;
}

int main_menu(const Screen scr) 
{
    #define MM_LEN 4
    static char *choices[MM_LEN] = {
        "Start new game",
        "Open a saving",
        "Create a new saving",
        "Quit"
    };

    return generic_menu(scr, 
        (StringArt){.art=choices, .length=MM_LEN}, 
        (StringArt){.art=_FROGGER_LOGO, .length=_FROGGER_LOGO_LENGTH}, 
        (StringArt){.art=_FROG_ART, .length=_FROG_ART_LENGTH});
}

int pause_menu(const Screen scr) 
{
    #define PM_LEN 6
    static char *choices[PM_LEN] = {
        "Resume",
        "Save current game",
        "Save as a Replay",
        "Create a new saving",
        "Save & Quit",
        "Quit"
    };

    return generic_menu(scr, 
        (StringArt){.art=choices, .length=PM_LEN}, 
        (StringArt){.art=_FROGGER_PAUSE_LOGO, .length=_FROGGER_PAUSE_LOGO_LENGTH}, 
        (StringArt){.art=_FROGGER_PAUSE_ART, .length=_FROGGER_PAUSE_LENGTH});
}

unsigned int show(const Screen scr, const enum PS prog_state,  int *output) 
{
    switch (prog_state) 
    {
        case PS_MAIN_MENU: 
            *output = main_menu(scr);
            break;
        case PS_PAUSE_MENU: 
            *output = pause_menu(scr);
            break;
        default: 
            return 1;
    }

    return 0;
}
