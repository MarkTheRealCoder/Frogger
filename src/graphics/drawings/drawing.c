#include "../drawing.h"

static Range timer;
static Range hps;
static Range achievements;
static Range trash_timers;
static Range map;

void init_screen(Screen *scrn) {
    setlocale(LC_ALL, "");
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


void configure_screen(Screen scr) {
    
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

void eraseFor(Position sp, short height, short length) {
    int pair = alloc_pair(COLOR_BLACK, COLOR_BLACK);

    attron(COLOR_PAIR(pair));
    for (short i = 0; i < length; i++) {
        for (short j = 0; j < height; j++) {
            mvaddch(sp.y+j, sp.x+i, ' ');
        }
    }
    attroff(COLOR_PAIR(pair));
}


void display_clock(const Position p, const short value, const short max) {
    int main_color = alloc_pair(COLOR_BLACK, COLOR_WHITE);
    int decaying_color = alloc_pair(COLOR_BLACK, COLOR_YELLOW);
    int decaying_2_color = alloc_pair(COLOR_BLACK, COLOR_RED);

    int parts_per_tick = (int) (max / 10);
    int ticks = (int) (value / parts_per_tick), left = (int) (value % parts_per_tick);
    int subticks = 0;
    if (left) subticks = (left > (int) (parts_per_tick / 2)) + 1;
    if (subticks == 2) left -= (int) (parts_per_tick / 2);

    eraseFor(p, 2, 20);

    for (int i = 0, color = 0; i < 20 && (!i || (int)(i/2) < ticks + (subticks > 0)); i+=2) {
        if ((!i && ticks) || (i && (int)(i/2) < ticks)) color = main_color;
        else if ((int)(left) > 2) color = decaying_color;
        else color = decaying_2_color;
        attron(COLOR_PAIR(color));
        mvaddch(p.y, p.x+i, ' ');
        mvaddch(p.y+1, p.x+i, ' ');
        if (color == main_color || subticks == 2) {
            mvaddch(p.y, p.x+i+1, ' ');
            mvaddch(p.y+1, p.x+i+1, ' ');
        }
        attroff(COLOR_PAIR(color));
    }
    refresh();
}

void print_hearts(int *x, int y, int curr, int max, int main_color, int lost_color) {
    for (int i = 0, pair; i < max; i++) {
        pair = (curr > i) ? main_color : lost_color;
        attron(COLOR_PAIR(pair));
        mvaddstr(y, *x, HEART);
        attroff(COLOR_PAIR(pair));
        (*x)++;
        mvaddch(y, *x, ' ');
        (*x)++;
    }
    refresh();
}

void display_hps(const Position p, const short mcurr, const short fcurr) {
    int mpair = alloc_pair(COLOR_RED, COLOR_BLACK);
    int fpair = alloc_pair(COLOR_GREEN, COLOR_BLACK);
    int lost = alloc_pair(COLOR_BLACK, COLOR_BLACK);

    int x = p.x;
    attron(A_BOLD);
    mvaddch(p.y, x, ' ');
    x++;
    print_hearts(&x, p.y, mcurr, MAIN_HPS, mpair, lost);
    mvaddch(p.y, x, ' ');
    x++;
    print_hearts(&x, p.y, fcurr, FROG_HPS, fpair, lost);
    attroff(A_BOLD);
    refresh();
}

void addStringToList(StringNode **list, int color, char *string) {
    StringNode *new = CALLOC(StringNode, 1);
    new->length = strlen(string);
    new->color = color;
    new->next = NULL;
    new->prev = NULL;
    new->string = string;

    if (*list) {
        (*list)->next = new;
        new->prev = (*list);
    }

    *list = new;
}

void display_achievements(const Position p, const short length, const short height, StringList list) {
    eraseFor(p, height, length);
    StringNode *last = list.last;
    for (int i = 0, lines = height; i < list.nodes; i++) {
        lines -= (int)(last->length / length) + ((last->length % length) > 0);
        if (lines == 0 || lines > 0) last = last->prev;
        else {
            last = last->next;
            break;
        }
    }

    if (last->next == NULL) return;

    const int x = p.x;
    int y = p.y;

    do {
        attron(COLOR_PAIR(last->color));
        for (int i = 0, increment = 0; i + increment < last->length; i++) {
            mvaddch(y, x+i, last->string[i + increment]);
            if (i == length) {
                increment = i-1;
                i = 0;
                y++;
            }
        }
        attroff(COLOR_PAIR(last->color));
        last = last->next;
    } while (last);
    refresh();
}




