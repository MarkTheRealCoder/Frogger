#include "../drawing.h"

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
 * Stampa una stringa al centro del terminale.
 * @param string La stringa da stampare.
 * @param pair Il colore da utilizzare.
 * @param max La larghezza della finestra
 */
void center_string_colored(char *string, int pair, int max, int cuy)
{
    attron(COLOR_PAIR(pair));
    center_string(string, max, cuy);
    attroff(COLOR_PAIR(pair));
    refresh();
}


/**
 * Stampa una stringa al centro del terminale.
 * @param str La stringa da stampare.
 * @param max La larghezza della finestra
 */
void center_string(char str[], int max, int cuy)
{
    int sl = strlen(str);
    int total_size = (int)((max - sl) / 2);
    move(cuy, 0);
    clrtoeol();
    mvaddstr(cuy, total_size, str); 
}





