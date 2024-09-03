#include "../drawing.h"
#include "../../utils/shortcuts.h"
#include "../../utils/globals.h"

static Range timer;
static Range hps;
static Range achievements;
static Range trash_timers;
static Range map;

/**
 * Inizializza lo schermo insieme ai colori.
 * @param screen Lo schermo da inizializzare.
 */
void init_screen(Screen *screen)
{
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    curs_set(false);
    keypad(stdscr, true);
    timeout(100);
    start_color();
    getmaxyx(stdscr, screen->y, screen->x);

    init_extended_color(COLORCODES_FROG_ART, FROG_ART_COLOR);
    init_extended_color(COLORCODES_FROG_ART_LOGO, FROG_ART_LOGO_COLOR);
    init_extended_color(COLORCODES_FROG_ART_LOGO_QUIT, FROG_ART_LOGO_COLOR_Q);
    init_extended_color(COLORCODES_FROG_ART_SELECTED, FROG_ART_SELECTED_COLOR);

    init_extended_color(COLORCODES_GRASS, GRASS_COLOR);
    init_extended_color(COLORCODES_BLOCKED, BLOCKED_COLOR);
    init_extended_color(COLORCODES_SIDEWALK, SIDEWALK_COLOR);
    init_extended_color(COLORCODES_WATER, WATER_COLOR);
    init_extended_color(COLORCODES_HIDEOUT, HIDEOUT_COLOR);

    init_extended_color(COLORCODES_CROC_B, CROC_B_COLOR);
    init_extended_color(COLORCODES_CROC_A, CROC_A_COLOR);
    init_extended_color(COLORCODES_FROG_B, FROG_B_COLOR);
    init_extended_color(COLORCODES_FROG_A, FROG_A_COLOR);
    init_extended_color(COLORCODES_PLANT_B, PLANT_B_COLOR);
    init_extended_color(COLORCODES_PLANT_A, PLANT_A_COLOR);
    init_extended_color(COLORCODES_PROJECTILE_F, PROJECTILE_F_COLOR);
    init_extended_color(COLORCODES_PROJECTILE_FL, PROJECTILE_FL_COLOR);
}

/**
 * Controlla la dimensione dello schermo ogni volta che viene ridimensionato.
 */
void handle_screen_resize() 
{
    #define MIN_ROWS 40
    #define MIN_COLS 100

    endwin();
    refresh();
    clear();
    
    int *xy = get_screen_size();
    int rows = xy[0], cols = xy[1];

    setScreenValidity(rows < MIN_ROWS || cols < MIN_COLS);

    if (!isScreenValid())
    {
        mvprintw(0, 0, "Screen size is too small to play Frogger! :(");
        mvprintw(1, 0, "Please resize the screen to at least 40rows x 100cols.");
    }
    
    refresh();
}

/**
 * Trova il colore corrispondente al tipo di entità.
 * @param type Il tipo di entità.
 * @return Il colore corrispondente.
 */
enum color_codes getEntityColor(const enum entity_type type)
{
    enum color_codes color;

    switch (type) 
    {
        case TRUETYPE_CROC:
            color = COLORCODES_CROC_B;
            break;
        case TRUETYPE_ANGRY_CROC:
            color = COLORCODES_CROC_A;
            break;
        case TRUETYPE_PLANT:
            color = COLORCODES_PLANT_B;
            break;
        case TRUETYPE_PLANT_HARMED:
            color = COLORCODES_PLANT_A;
            break;
        case TRUETYPE_FROG:
            color = COLORCODES_FROG_B;
            break;
        case TRUETYPE_FROG_HARMED:
            color = COLORCODES_FROG_A;
            break;
        case TRUETYPE_PROJ_FROG:
            color = COLORCODES_PROJECTILE_F;
            break;
        case TRUETYPE_PROJ_PLANT:
            color = COLORCODES_PROJECTILE_FL;
            break;
    }

    return color;
}

/**
 * Trova il colore dell'area della riga basandosi sulla coordinata Y.
 * @param y La coordinata Y.
 * @return Il colore corrispondente.
 */
int getAreaFromY(const int y) 
{
    int areaColor = COLOR_BLACK;

    if (y < 3) areaColor = COLORCODES_BLOCKED;
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

/**
 * Rimpiazza un'area dello schermo con un carattere.
 * @param pair Il colore da utilizzare.
 * @param sp La posizione iniziale.
 * @param height L'altezza dell'area.
 * @param length La lunghezza dell'area.
 * @param c Il carattere da stampare.
 */
void replaceWith(int pair, Position sp, int height, int length, char c) 
{
    attron(COLOR_PAIR(pair));

    for (short i = 0; i < length; i++) 
    {
        for (short j = 0; j < height; j++) 
        {
            mvaddch(sp.y + j, sp.x + i, c);
        }
    }

    attroff(COLOR_PAIR(pair));
}

/**
 * Pulisce un'area dello schermo.
 * @param sp La posizione iniziale.
 * @param height L'altezza dell'area.
 * @param length La lunghezza dell'area.
 */
void eraseFor(Position sp, short height, short length) 
{
    int pair = alloc_pair(COLOR_BLACK, COLOR_BLACK);
    replaceWith(pair, sp, height, length, ' ');
}

/**
 * Stampa a schermo il tempo rimanente.
 * @param p La posizione in cui stampare.
 * @param value Il valore da stampare.
 * @param max Il valore massimo.
 */
void display_clock(const Position p, const short value, const short max) 
{
    int main_color = alloc_pair(COLOR_BLACK, COLOR_WHITE);
    int decaying_color = alloc_pair(COLOR_BLACK, COLOR_YELLOW);
    int decaying_2_color = alloc_pair(COLOR_BLACK, COLOR_RED);

    int parts_per_tick = (int) (max / 10);
    int ticks = (int) (value / parts_per_tick), left = (int) (value % parts_per_tick);
    int subticks = 0;

    if (left) 
    {
        subticks = (left > (int) (parts_per_tick / 2)) + 1;
    }

    if (subticks == 2) 
    {
        left -= (int) (parts_per_tick / 2);
    }

    eraseFor(p, 2, 20);

    for (int i = 0, color = 0; i < 20 && (!i || (int)(i / 2) < ticks + (subticks > 0)); i += 2) 
    {
        if ((!i && ticks) || (i && (int)(i / 2) < ticks)) 
        {
            color = main_color;
        }
        else if ((int)(left) > 2)
        {
            color = decaying_color;
        }
        else 
        {
            color = decaying_2_color;
        }

        attron(COLOR_PAIR(color));

        int tempX = p.x + i;

        mvaddch(p.y, tempX, ' ');
        mvaddch(p.y + 1, tempX, ' ');
        
        if (color == main_color || subticks == 2) 
        {
            mvaddch(p.y, tempX + 1, ' ');
            mvaddch(p.y + 1, tempX + 1, ' ');
        }
        
        attroff(COLOR_PAIR(color));
    }

    refresh();
}

/**
 * Stampa a schermo i cuori.
 * @param x La posizione X in cui stampare.
 * @param y La posizione Y in cui stampare.
 * @param curr Il valore attuale.
 * @param max Il valore massimo.
 * @param main_color Il colore principale.
 * @param lost_color Il colore per i cuori persi.
 */
void print_hearts(int *x, int y, int curr, int max, int main_color, int lost_color)
{
    for (int i = 0, pair; i < max; i++)
    {
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

/**
 * Stampa a schermo i cuori per i punti vita.
 * @param position La posizione in cui stampare.
 * @param lives I punti vita attuali del giocatore.
 */
void display_hps(const Position position, const short lives)
{
    int fpair = alloc_pair(COLOR_GREEN, COLOR_BLACK);
    int lost = alloc_pair(COLOR_BLACK, COLOR_BLACK);

    int x = position.x;
    int y = position.y;

    attron(A_BOLD);
    mvaddch(y, x++, ' ');

    print_hearts(&x, y, lives, TOTAL_LIVES, fpair, lost);
    refresh();
}

/**
 * Stampa a schermo il punteggio.
 * @param position La posizione in cui stampare.
 * @param score Il punteggio attuale del giocatore.
 */
void display_score(const Position position, const int score)
{
    #define MAX_SIZE 7 + 8

    char stringScore[MAX_SIZE];
    snprintf(stringScore, sizeof(stringScore), "Score: %d", score);

    display_string(position, COLOR_CYAN, stringScore, MAX_SIZE);
}

/**
 * Aggiunge una stringa alla lista.
 * @param list La lista.
 * @param color Il colore della stringa.
 * @param string La stringa da aggiungere.
 */
void addStringToList(StringNode **list, int color, char *string) 
{
    StringNode *new = CALLOC(StringNode, 1);
    new->length = strlen(string);
    new->color = color;
    new->next = NULL;
    new->prev = NULL;
    new->string = string;

    if (*list) 
    {
        (*list)->next = new;
        new->prev = (*list);
    }

    *list = new;
}

void display_string(const Position position, const int color, const char *string, int length)
{
    eraseFor(position, 1, length);
    
    int pair = alloc_pair(color, getAreaFromY(position.y));

    attron(COLOR_PAIR(pair));
    mvaddstr(position.y, position.x, string);
    attroff(COLOR_PAIR(pair));
    
    refresh();
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

void display_entity(const int fg, const StringArt art, const Position curr, const Position last, const MapSkeleton map) {
    const int pair = alloc_pair(fg, getAreaFromY(GET_MAP_Y(curr.y, map)));
    const int old_pair = alloc_pair(COLOR_BLACK, getAreaFromY(GET_MAP_Y(last.y, map)));
    const int length = strlen(art.art[0]);
    const int height = art.length; 

    attron(COLOR_PAIR(old_pair));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < length; j++) {
            if (WITHIN_BOUNDARIES(last.x+j, last.y+i, map)) {
                mvaddch(last.y+i, last.x+j, ' ');
            }
        }
    }
    attroff(COLOR_PAIR(old_pair));
    attron(COLOR_PAIR(pair));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < length; j++) {
            if (WITHIN_BOUNDARIES(curr.x+j, curr.y+i, map)) {
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
    map->width = width;
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

static bool GLOBAL_SCREEN_INVALID_SIZE = false;

/**
 * Imposta la validità dello schermo.
 * @param value Il valore da impostare.
 */
void setScreenValidity(bool value)
{
    GLOBAL_SCREEN_INVALID_SIZE = value;
}

/**
 * Controlla se lo schermo è valido.
 * @return  Se lo schermo è valido.
 */
bool isScreenValid()
{
    return !GLOBAL_SCREEN_INVALID_SIZE;
}

void draw(struct entity_node *es, MapSkeleton *map, Clock *timer, StringList *achievements, int score, int lives, bool drawAll)
{
    const static Position POSITION_MAP = {
        getCenteredX(MAP_WIDTH),
        5
    };

    const static Position POSITION_ACHIEVEMENTS_TITLE = {
        getCenteredX(12) + 72,
        getCenteredY(25) - 2
    };

    const static Position POSITION_ACHIEVEMENTS = {
        getCenteredX(30) + 75,
        getCenteredY(25)
    };

    const static Position POSITION_LIVES = {
        getCenteredX(TOTAL_LIVES) - 45,
        3
    };

    const static Position POSITION_SCORE = {
        getCenteredX(12),
        3
    };

    const static Position POSITION_CLOCK = {
        getCenteredX(0) + 25,
        2
    };

    if (drawAll)
    {
        display_map(POSITION_MAP, MAP_WIDTH, map);
        display_string(POSITION_ACHIEVEMENTS_TITLE, COLOR_RED, "Achievements", 12);
    }

    display_hps(POSITION_LIVES, lives);
    display_score(POSITION_SCORE, score);

    display_clock(POSITION_CLOCK, timer->current, timer->starting);
    display_achievements(POSITION_ACHIEVEMENTS, 34, 25, *achievements);

    for (int i = 0; i < 5; i++) {
        struct entity_node *ec = es;
        while (ec != NULL) {
            if (getPriorityByEntityType(ec->entity.type) == i) {
                /*Bisogna modificare la struct entity: bisogna inserire più informazioni, come il tipo dell'entità (DISPLAY) e la vecchia posizione*/
                display_entity(COLORCODES_CROC_A, getArtOfEntity(&ec->entity), (Position){ec->entity.x, ec->entity.y}, (Position){ec->entity.x, ec->entity.y}, *map);
            }
            ec = ec->next;
        }
    }
}