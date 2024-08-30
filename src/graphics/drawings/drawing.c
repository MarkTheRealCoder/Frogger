#include "../drawing.h"
#include "../../utils/shortcuts.h"

static Range timer;
static Range hps;
static Range achievements;
static Range trash_timers;
static Range map;

void init_screen(Screen *scrn) 
{
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

    init_extended_color(COLORCODES_GRASS, GRASS_COLOR);
    init_extended_color(COLORCODES_SIDEWALK, SIDEWALK_COLOR);
    init_extended_color(COLORCODES_WATER, WATER_COLOR);
    init_extended_color(COLORCODES_HIDEOUT, HIDEOUT_COLOR);
}


void configure_screen(Screen scr) {
    
}

enum color_codes getEntityColor(const enum entity_type type) 
{
    enum color_codes color;

    switch (type) 
    {
        case CROC_TYPE:
            color = COLORCODES_CROC_B;
            break;
        case CROC_ANGRY_TYPE: 
            color = COLORCODES_CROC_A;
            break;
        case FLOWER_TYPE:
            color = COLORCODES_FLOWER_B;
            break;
        case FLOWER_HARMED_TYPE: 
            color = COLORCODES_FLOWER_A;
            break;
        case FROG_TYPE: 
            color = COLORCODES_FROG_B;
            break;
        case FROG_HARMED_TYPE:
            color = COLORCODES_FROG_A;
            break;
        case PROJ_FROG_TYPE: 
            color = COLORCODES_PROJECTILE_F;
            break;
        case PROJ_FLOWER_TYPE: 
            color = COLORCODES_PROJECTILE_FL;
            break;
    }

    return color;
}

int getAreaFromY(const int y) {
    int areaColor = COLOR_BLACK;
    if (y < 3) areaColor = COLORCODES_GRASS;
    else if (y < 6) areaColor = COLORCODES_HIDEOUT;
    else if (y < 12) areaColor = COLORCODES_GRASS;
    else if (y < 36) areaColor = COLORCODES_WATER;
    else if (y < 39) areaColor = COLORCODES_SIDEWALK;
    return areaColor;
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

void replaceWith(int pair, Position sp, int height, int length, char c) {
    attron(COLOR_PAIR(pair));
    for (short i = 0; i < length; i++) {
        for (short j = 0; j < height; j++) {
            mvaddch(sp.y+j, sp.x+i, c);
        }
    }
    attroff(COLOR_PAIR(pair));
}

void eraseFor(Position sp, short height, short length) {
    int pair = alloc_pair(COLOR_BLACK, COLOR_BLACK);
    replaceWith(pair, sp, height, length, ' ');
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
        if (lines == 0 || lines > 0) {
            if (last->prev) last = last->prev;
            else break;
        }
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
            if (i + 1 == length) {
                increment += i + 1;
                i = -1;
                y++;
            }
        }
        y++;
        attroff(COLOR_PAIR(last->color));
        last = last->next;
    } while (last);
    refresh();
}

void display_entity(const int fg, const StringArt art, const Position curr, const Position last) {
    const int pair = alloc_pair(fg, getAreaFromY(curr.y));
    const int old_pair = alloc_pair(COLOR_BLACK, getAreaFromY(last.y));
    const int length = strlen(art.art[0]);
    const int height = art.length; 

    attron(COLOR_PAIR(old_pair));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < length; j++) {
            if (WITHIN_BOUNDARIES(1) /*ADAPT WITH MAP BOUNDARIES*/) {
                mvaddch(last.y+i, last.x+j, ' ');
            }
        }
    }
    attroff(COLOR_PAIR(old_pair));
    attron(COLOR_PAIR(pair));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < length; j++) {
            if (WITHIN_BOUNDARIES(1) /*ADAPT WITH MAP BOUNDARIES*/) {
                mvaddch(curr.y+i, curr.x+j, art.art[i][j]);
            }
        }
    }
    attroff(COLOR_PAIR(pair));
    refresh();
}


/* * * * * * */

void make_MapSkeleton(MapSkeleton *map, const Position sp, const int width) {
    int hideouts = HIDEOUTS;
    while (!(width > hideouts * FROG_WIDTH)) hideouts--;
    map->hideouts = CALLOC(Position, hideouts + 1);
    int total_left = width - hideouts * FROG_WIDTH;
    int spaces = hideouts + 1;
    int left_per_space = (int) (total_left / spaces);
    int extra_left = total_left % spaces;
    extra_left = extra_left % 2 == 0 ? extra_left : extra_left - 1;
    spaces = spaces % 2 == 0;
    for (int i = 0, spacing = 0; i < hideouts; i++) {
        map->hideouts[i].y = sp.y + FROG_HEIGHT; 
        spacing += left_per_space;
        if (extra_left) {
            if (spaces && i + 1 == ((int) (hideouts / 2)) + 1) spacing += extra_left;
            else if (i + 1 == ((int) (hideouts / 2)) + 1 || i + 1 == ((int) (hideouts / 2)) + 2) spacing += (int) (extra_left / 2);
        }
        map->hideouts[i].x = sp.x + spacing;
        spacing += FROG_WIDTH;
    }
    map->hideouts[hideouts].y = map->hideouts[hideouts].x = -1;


    map->garden.x = map->sidewalk.x = map->river.x = sp.x;
    map->garden.y = map->hideouts[0].y + FROG_HEIGHT;
    map->river.y = map->garden.y + FROG_HEIGHT * 2;
    map->sidewalk.y = map->river.y + FROG_HEIGHT * 8;
}

MapSkeleton display_map(const Position sp, const int width, MapSkeleton* map) {
    int current_pair = -1;
    MapSkeleton _map; 

    if (!map) make_MapSkeleton(&_map, sp, width);
    else _map = *map;

    for (int i = 0, current_bg = -1, tmp_bg = -1, tmp_pair = 0; i < MAP_HEIGHT; i++) {
        tmp_bg = getAreaFromY(i);

        if (tmp_bg != current_bg && (i < 3 || i > 5)) {
            current_bg = tmp_bg;
            if (current_pair != -1) attroff(COLOR_PAIR(current_pair));
            current_pair = alloc_pair(COLOR_BLACK, current_bg);
            attron(COLOR_PAIR(current_pair));
        }
        else if (tmp_bg != current_bg) {
            tmp_pair = alloc_pair(COLOR_BLACK, tmp_bg);
        }
        
        
        for (int j = 0, k = 0; j < width; j++) {
            if (sp.y + i >= _map.hideouts[0].y && sp.y + i < _map.hideouts[0].y + FROG_HEIGHT) {
                bool ho = false;
                if (sp.x + j >= _map.hideouts[k].x && sp.x + j < _map.hideouts[k].x + FROG_WIDTH) {
                    ho = true;
                    if (sp.x + j == _map.hideouts[k].x + FROG_WIDTH - 1) k++;
                }
                if (ho) {
                    attroff(COLOR_PAIR(current_pair));
                    attron(COLOR_PAIR(tmp_pair));
                }
                else {
                    attroff(COLOR_PAIR(tmp_pair));
                    attron(COLOR_PAIR(current_pair));
                }
            }
            mvaddch(sp.y + i, sp.x + j, ' ');
        }
    }
    attroff(COLOR_PAIR(current_pair));
    refresh();

    return _map;
}




