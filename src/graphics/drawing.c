#include "drawing.h"


void init_screen(Screen *scrn) {
    initscr();
    noecho();
    curs_set(false);
    keypad(stdscr, true);
    start_color();
    getmaxyx(stdscr, scrn->y, scrn->x);

    init_extended_color(COLORCODES_FROG_ART, FROG_ART_COLOR);
    init_extended_color(COLORCODES_FROG_ART_LOGO, FROG_ART_LOGO_COLOR);
    init_extended_color(COLORCODES_FROG_ART_LOGO_QUIT, FROG_ART_LOGO_COLOR_Q);
    init_extended_color(COLORCODES_FROG_ART_SELECTED, FROG_ART_SELECTED_COLOR);
}

enum color_codes getEntityColor(const enum entity_type type) {
    enum color_codes color;
    switch (type) {
        case CROC_TYPE: color = COLORCODES_CROC_B;
            break;
        case CROC_ANGRY_TYPE: color = COLORCODES_CROC_A;
            break;
        case FLOWER_TYPE: color = COLORCODES_FLOWER_B;
            break;
        case FLOWER_HARMED_TYPE: color = COLORCODES_FLOWER_A;
            break;
        case FROG_TYPE: color = COLORCODES_FROG_B;
            break;
        case FROG_HARMED_TYPE: color = COLORCODES_FROG_A;
            break;
        case PROJ_FROG_TYPE: color = COLORCODES_PROJECTILE_F;
            break;
        case PROJ_FLOWER_TYPE: color = COLORCODES_PROJECTILE_FL;
            break;
    }
    return color;
}

enum areas getAreaFromPos(const Position pos) {
    
}

void eraseOldPosition(const enum color_codes areaColor, const Position old, const unsigned int length) {
    init_extended_pair(JOLLY_PAIR, 0, areaColor);
    attron(COLOR_PAIR(JOLLY_PAIR));
    for (int i = 0; i < length; i++) {
        mvaddch(old.y, old.x + i, ' ');
        mvaddch(old.y+1, old.x + i, ' ');
        mvaddch(old.y+2, old.x + i, ' ');
    }
    attroff(COLOR_PAIR(JOLLY_PAIR));
}

EntityObject getEntityObject(const enum entity_type type) {
    return (EntityObject){.top = NULL, .middle = NULL, .bottom = NULL};
}

void drawNewEntity(const enum entity_type type, const enum color_codes areaColor, const Position new, const unsigned int length) {
    init_extended_pair(JOLLY_PAIR, getEntityColor(type), areaColor);
    EntityObject obj = getEntityObject(type);
    attron(COLOR_PAIR(JOLLY_PAIR));
    //todo Replace following
    mvaddstr(new.y, new.x, obj.top);
    mvaddstr(new.y+1, new.x, obj.middle);
    mvaddstr(new.y+2, new.x, obj.bottom);
    //
    attroff(COLOR_PAIR(JOLLY_PAIR));
}

void move_entity(const enum entity_type type, const Position old, const Position current, const unsigned int length) {
    enum areas oldArea = getAreaFromPos(old);
    enum areas newArea = getAreaFromPos(current);
    eraseOldPosition((enum color_codes)oldArea, old, length);
    drawNewEntity(type, (enum color_codes)newArea, current, length);
    refresh();
}

/**
 * Menu' di selezione della modalita' da utilizzare per il gioco.
 * @param max La larghezza della finestra.
 * @param may L'altezza della finestra.
 * @param current_choice L'indice corrente del menu'.
 */
void send_menu(int may, int max, int current_choice)
{
    erase();

    init_extended_pair(JOLLY_PAIR, COLORCODES_FROG_ART, COLOR_BLACK);
    attroff(COLOR_PAIR(JOLLY_PAIR));
    attron(COLOR_PAIR(JOLLY_PAIR) | A_BOLD);
    for (size_t i = 0; i < _FROG_ART_LENGTH; i++)
    {
        center_string(_FROG_ART[i], max);
    }
    attroff(COLOR_PAIR(JOLLY_PAIR) | A_BOLD);

    print_logo(COLORCODES_FROG_ART_LOGO, max);
    // x0, y19
    print_choices(current_choice, max);
    refresh();
}

/**
 * Stampa il logo del gioco.
 * @param color Il colore da utilizzare.
 * @param max La larghezza della finestra
 */
void print_logo(int color, int max)
{
    init_extended_pair(JOLLY_PAIR, color, COLOR_BLACK);
    attroff(COLOR_PAIR(JOLLY_PAIR));
    attron(COLOR_PAIR(JOLLY_PAIR) | A_BOLD);
    for (size_t i = 0; i < _FROGGER_LOGO_LENGTH; i++)
    {
        center_string(_FROGGER_LOGO[i], max);
    }
    attroff(COLOR_PAIR(JOLLY_PAIR) | A_BOLD);
    refresh();
}

/**
 * Stampa le scelte del menu'.
 * @param current_choice L'indice corrente del menu'.
 * @param max La larghezza della finestra.
 */
void print_choices(int current_choice, int max)
{
    for (size_t i = 0; i < _MENU_CHOICES_LENGTH; i++)
    {
        if (current_choice == i)
        {
            char *tmp = concat(3, "> ", _MENU_CHOICES[i], " <");

            sprint_colored(tmp, COLORCODES_FROG_ART_SELECTED, max);
            free(tmp);
            continue;
        }

        sprint_colored(_MENU_CHOICES[i], i == (_MENU_CHOICES_LENGTH - 1) ? COLORCODES_FROG_ART_LOGO_QUIT : COLORCODES_FROG_ART_LOGO, max);
    }
}

/**
 * Stampa una stringa al centro del terminale.
 * @param string La stringa da stampare.
 * @param color Il colore da utilizzare.
 * @param max La larghezza della finestra
 */
void sprint_colored(char *string, int color, int max)
{
    init_extended_pair(JOLLY_PAIR, color, COLOR_BLACK);
    attroff(COLOR_PAIR(JOLLY_PAIR));
    attron(COLOR_PAIR(JOLLY_PAIR));
    center_string(string, max);
    attroff(COLOR_PAIR(JOLLY_PAIR));
    refresh();
}


/**
 * Stampa una stringa al centro del terminale.
 * @param str La stringa da stampare.
 * @param max La larghezza della finestra
 */
void center_string(char str[], int max)
{
    printf("%s\n", str);
    int sl = strlen(str);
    int total_size = (int)((max - sl) / 2);

    for (int i = 0; i < total_size; i++)
    {
        addch(' ');
    }

    addstr(str);
}

void menu_listener(int *choice) {
    wgetch(stdscr); //TODO: adapt this for the user's allowed keys.
    *choice = -1;
}

void main_menu(const Screen scr) {
    int choice = 1;
    do {
        send_menu(scr.y, scr.x, choice-1);
        menu_listener(&choice);
    } while(choice >= 1);
    return (-choice) - 1;
}

void game(const Screen scr) {

}

void pause_menu(const Screen scr) {

}


unsigned int show(const Screen scr, const enum PS prog_state) {
    switch (prog_state) {
        case PS_MAIN_MENU: main_menu(scr);
            break;

        case PS_GAME: game(scr);
            break;
        
        case PS_PAUSE_MENU: pause_menu(scr);
            break;
        
        default: return 1;
    }
    return 0;
}





