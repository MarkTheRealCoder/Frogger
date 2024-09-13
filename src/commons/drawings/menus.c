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
    int starting_choice = 0;
    for (size_t i = starting_choice; i < choices_num; i++)
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
    clear_screen();

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

    clear_screen();

    return choice;
}

int main_menu(const Screen screen)
{
    #define MENU_MAIN_LEN 2
    static char *choices[MENU_MAIN_LEN] = {
        "Start new game",
        "Quit"
    };

    return generic_menu(screen, getArtOfThing(ART_UNKNOWN , choices, MENU_MAIN_LEN),
                                getArtOfThing(ART_MAIN_LOGO, NULL, 0),
                                getArtOfThing(ART_BIG_FROG, NULL, 0));
}

int pause_menu(const Screen screen)
{
    #define MENU_PAUSE_LEN 2
    static char *choices[MENU_PAUSE_LEN] = {
        "Resume",
        "Quit"
    };

    return generic_menu(screen, getArtOfThing(ART_UNKNOWN, choices, MENU_PAUSE_LEN),
                                getArtOfThing(ART_PAUSE_LOGO, NULL, 0),
                                getArtOfThing(ART_TWO_FROGS, NULL, 0));
}

int thread_or_processes_menu(const Screen screen)
{
    #define MENU_THREAD_OR_PROCESSES_LEN 3
    static char *choices[MENU_THREAD_OR_PROCESSES_LEN] = {
        "Threads version",
        "Processes version",
        "Quit"
    };

    return generic_menu(screen, getArtOfThing(ART_UNKNOWN, choices, MENU_THREAD_OR_PROCESSES_LEN),
                                getArtOfThing(ART_MAIN_LOGO, NULL, 0),
                                getArtOfThing(ART_BIG_FROG, NULL, 0));
}

int game_over_menu(const Screen screen, const enum PS state, const int score)
{
    #define MENU_GAME_OVER_LEN 2

    static char *choices[MENU_GAME_OVER_LEN] = {
            "Play Again",
            "Quit"
    };

    static char *partOfMessage = "You scored: ";

    enum AVAILABLE_ARTS art = state == PS_LOST ? ART_LOST_LOGO : ART_WIN_LOGO;

    char *scoreString = numToString(score);
    char *scoreMessage = concat(2, partOfMessage, scoreString);

    int menuResult = generic_menu(screen, getArtOfThing(ART_UNKNOWN, choices, MENU_GAME_OVER_LEN),
                                  getArtOfThing(ART_UNKNOWN, &scoreMessage, 1),
                                  getArtOfThing(art, NULL, 0));
    free(scoreMessage);
    free(scoreString);

    return menuResult;
}

unsigned int show(const Screen screen, const enum PS state, int *output)
{
    clear_screen();

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
        case PS_WIN:
        case PS_LOST:
            *output = game_over_menu(screen, state, *output);
            break;
        default: 
            return 1;
    }

    clear_screen();
    return 0;
}
