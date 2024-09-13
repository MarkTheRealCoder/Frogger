#include "../drawing.h"

void print_art(StringArt art, int *cuy, int max)
{
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
 */
void print_choices(char **choices, int choices_num, int current_choice, int max, int *cuy)
{
    int standard    = alloc_pair(COLORCODES_FROG_ART_LOGO, COLOR_BLACK);
    int quit        = alloc_pair(COLORCODES_FROG_ART_LOGO_QUIT, COLOR_BLACK);
    int selected    = alloc_pair(COLORCODES_FROG_ART_SELECTED, COLOR_BLACK);
    int starting_choice = current_choice <= 0 ? 0 : current_choice > choices_num - 3 && choices_num - 3 >= 0 ? choices_num - 3 : current_choice - 1;
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
        default:
            break;
    }

    return not_exit;
}

int generic_menu(const Screen screen, StringArt choices, StringArt logo, StringArt art)
{
    erase();

    int standard = alloc_pair(COLORCODES_FROG_ART_LOGO, COLOR_BLACK);
    int quit = alloc_pair(COLORCODES_FROG_ART_LOGO_QUIT, COLOR_BLACK);

    int choice = 0;
    int cuy = 1;

    print_art(art, &cuy, screen.x);
    cuy++;
    int logo_cuy = cuy;

    do 
    {
        cuy = logo_cuy;
        print_logo(logo, choice == choices.length - 1 ? quit : standard, screen.x, &cuy);

        cuy += 4;
        print_choices(choices.art, choices.length, choice, screen.x, &cuy);
    } while (menu_listener(&choice, choices.length));

    erase();

    return choice;
}

int main_menu(const Screen screen)
{
    #define MM_LEN 2
    static char *choices[MM_LEN] = {
        "Start new game",
        "Quit"
    };

    return generic_menu(screen, getArtOfThing(ART_UNKNOWN ,choices, MM_LEN),
                                getArtOfThing(ART_MAIN_LOGO, _FROGGER_LOGO, 0),
                                getArtOfThing(ART_BIG_FROG, _FROG_ART, 0));
}

int pause_menu(const Screen screen)
{
    #define PM_LEN 2
    static char *choices[PM_LEN] = {
        "Resume",
        "Quit"
    };

    return generic_menu(screen, getArtOfThing(ART_UNKNOWN, choices, MM_LEN),
                                getArtOfThing(ART_PAUSE_LOGO, _FROGGER_PAUSE_LOGO, 0),
                                getArtOfThing(ART_TWO_FROGS, _FROGGER_PAUSE_ART, 0));
}

int thread_or_processes_menu(const Screen screen)
{
    #define TOPM_LEN 3
    static char *choices[TOPM_LEN] = {
        "Threads version",
        "Processes version",
        "Quit"
    };

    return generic_menu(screen, getArtOfThing(ART_UNKNOWN, choices, TOPM_LEN),
                                getArtOfThing(ART_MAIN_LOGO, _FROGGER_LOGO, 0),
                                getArtOfThing(ART_BIG_FROG, _FROG_ART, 0));
}

unsigned int show(const Screen screen, const enum PS state, int *output)
{
    erase();

    switch (state)
    {
        case PS_MAIN_MENU: 
            *output = main_menu(screen);
            break;
        case PS_PAUSE_MENU: 
            *output = pause_menu(screen);
            break;
        case PS_VERSION_MENU:
            *output = thread_or_processes_menu(screen);
            break;
        default: 
            return 1;
    }

    erase();
    return 0;
}
