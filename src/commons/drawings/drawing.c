#include "../drawing.h"

void clear_screen()
{
    attrset(A_NORMAL);
    clear();
    erase();
    refresh();
}

/**
 * Inizializza lo schermo insieme ai colori.
 * @param screen    Lo schermo da inizializzare.
 * @return          La finestra inizializzata.
 */
WINDOW *init_screen(Screen *screen)
{
    setlocale(LC_ALL, "");
    WINDOW *w = initscr();
    noecho();
    cbreak();
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
    init_extended_color(COLORCODES_PLANT_B, PLANT_B_COLOR);
    init_extended_color(COLORCODES_PLANT_A, PLANT_A_COLOR);
    init_extended_color(COLORCODES_PROJECTILE_F, PROJECTILE_F_COLOR);
    init_extended_color(COLORCODES_PROJECTILE_FL, PROJECTILE_FL_COLOR);

    return w;
}

/**
 * Controlla la dimensione dello schermo ogni volta che viene ridimensionato.
 */
void handle_screen_resize()
{
    static bool SCREEN_INVALID = true;

    #define MIN_ROWS 42
    #define MIN_COLS 100

    int *xy = get_screen_size();
    int rows = xy[0], cols = xy[1];

    SCREEN_INVALID = rows < MIN_ROWS || cols < MIN_COLS;

    if (SCREEN_INVALID)
    {
        erase();
        refresh();
        wgetch(stdscr);
        endwin();

        printf("\n\nFrogger needs a minimum display size of %d x %d to play!\n", MIN_ROWS, MIN_COLS);
        printf("Current display size: %d x %d\n\n", cols, rows);
        exit(EXIT_FAILURE);
    }
}

/**
 * Trova il colore corrispondente al tipo di entità.
 * @param type  Il tipo di entità.
 * @return      Il colore corrispondente.
 */
enum color_codes getEntityColor(const TrueType type)
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
 * Prende la posizione dell'entità.
 * @param entity    L'entità.
 * @return          La posizione dell'entità.
 */
Position getPositionFromEntity(const Entity entity)
{
    return entity.current;
}

/**
 * Crea una posizione basandosi sulle coordinate.
 * @param x La coordinata X.
 * @param y La coordinata Y.
 * @return  La posizione delle coordinate.
 */
Position getPosition(const int x, const int y)
{
    return (Position) { .x = x, .y = y };
}

/**
 * Ritorna l'altezza dell'entita' in base al suo tipo.
 * @param entityType    Il tipo dell'entita'.
 * @return              L'altezza dell'entita'.
 */ 
int getHeightByEntityType(const EntityType entityType)
{
    return entityType == ENTITY_TYPE__PROJECTILE ? 1 : FROG_HEIGHT;
}

/**
 * Ritorna la priorita' dell'entita' in base al suo tipo.
 * @param entityType    Il tipo dell'entita'.
 * @return              La priorita' dell'entita'.
 */
int getPriorityByEntityType(const EntityType entityType)
{
    int prio = 0; 

    switch (entityType) 
    {
        case ENTITY_TYPE__PLANT: 
            prio = 3;
            break;
        case ENTITY_TYPE__PROJECTILE: 
            prio = 4;
            break;
        case ENTITY_TYPE__CROC: 
            prio = 1;
            break;
        case ENTITY_TYPE__FROG: 
            prio = 2;
            break;
        case ENTITY_TYPE__EMPTY: 
            prio = 0;
            break;
    }

    return prio;
}

/**
 * Crea un cuboide a partire da una posizione, una larghezza e un'altezza.
 * @param leftCorner    La posizione dell'angolo in basso a sinistra del cuboide.
 * @param width         La larghezza del cuboide.
 * @param height        L'altezza del cuboide.
 * @return              Il cuboide creato.
 */
Cuboid createCuboid(const Position leftCorner, const int width, const int height)
{
    const Cuboid cuboid = {
        .leftcorner = leftCorner,
        .rightcorner = {
            .x = leftCorner.x + width - 1,
            .y = leftCorner.y + height - 1
        }
    };

    return cuboid;
}

/**
 * Confronta due cuboidi..
 * @param c1    Il primo cuboide.
 * @param c2    Il secondo cuboide.
 * @return      True se i cuboidi si sovrappongono, false altrimenti.
 */
bool compareCuboids(const Cuboid c1, const Cuboid c2)
{
    for (int x = c2.leftcorner.x; x <= c2.rightcorner.x; x++)
    {
        if (!(c1.leftcorner.x <= x && x <= c1.rightcorner.x))
        {
            continue;
        }

        for (int y = c2.leftcorner.y; y <= c2.rightcorner.y; y++) 
        {
            if (c1.leftcorner.y <= y && y <= c1.rightcorner.y) 
            {
                return true;
            }
        }
    }

    return false;
}

/**
 * Crea un cuboide a partire da una struct entity.
 * @param e L'entita' da cui estrarre il cuboide.
 * @return  Il cuboide creato.
 */
Cuboid getCuboidFromEntity(const Entity e)
{
    return createCuboid(getPositionFromEntity(e), e.width, getHeightByEntityType(e.type));
}

/**
 * Controlla se due entita' stanno collidendo.
 * @param e1 La prima entita'.
 * @param e2 La seconda entita'.
 */
CollisionPacket areColliding(const Entity e1, const Entity e2)
{
    CollisionPacket collisionPacket = {
        .e1 = e1.trueType,
        .e2 = e2.trueType,
        .e1_priority = getPriorityByEntityType(e1.type),
        .e2_priority = getPriorityByEntityType(e2.type)
    };

    Cuboid c1 = getCuboidFromEntity(e1);
    Cuboid c2 = getCuboidFromEntity(e2);

    if (!compareCuboids(c1, c2)) 
    {
        collisionPacket.collision_type = COLLISION_AVOIDED;
    }
    else 
    {
        collisionPacket.collision_type = e1.trueType & e2.trueType;
    }

    return collisionPacket;
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
    if (!str)
    {
        return;
    }

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

    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < height; j++)
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
void eraseFor(Position sp, int height, int length)
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
void display_clock(const Position p, const int value, const unsigned int max)
{
    int main_color = alloc_pair(COLOR_BLACK, COLOR_WHITE);
    int decaying_color = alloc_pair(COLOR_BLACK, COLOR_YELLOW);
    int decaying_2_color = alloc_pair(COLOR_BLACK, COLOR_RED);

    int parts_per_tick = (int) (max / CLOCK_DISPLAY_TRUE_SIZE);
    int ticks = (int) (value / parts_per_tick), left = (int) (value % parts_per_tick);
    int subticks = 0;

    if (left) {
        subticks = (left > (int) (parts_per_tick / 2)) + 1;
    }

    if (subticks == 2) {
        left -= (int) (parts_per_tick / 2);
    }

    eraseFor(p, 2, CLOCK_DISPLAY_SIZE);

    for (int i = 0, color = 0; i < CLOCK_DISPLAY_SIZE && (!i || (int)(i / 2) < ticks + (subticks > 0)); i += 2)
    {
        if ((!i && ticks) || (i && (int)(i / 2) < ticks)) {
            color = main_color;
        }
        else if ((int)(left) > 2) {
            color = decaying_color;
        }
        else {
            color = decaying_2_color;
        }

        attron(COLOR_PAIR(color));

        int tempX = p.x + i;

        mvaddch(p.y, tempX, ' ');
        mvaddch(p.y + 1, tempX, ' ');
        
        if (color == main_color || subticks == 2) {
            mvaddch(p.y, tempX + 1, ' ');
            mvaddch(p.y + 1, tempX + 1, ' ');
        }
        
        attroff(COLOR_PAIR(color));
    }

    refresh();
}

/**
 * Stampa a schermo i cuori.
 * @param x             La posizione X in cui stampare.
 * @param y             La posizione Y in cui stampare.
 * @param curr          Il valore attuale.
 * @param max           Il valore massimo.
 * @param main_color    Il colore principale.
 * @param lost_color    Il colore per i cuori persi.
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
 * @param position  La posizione in cui stampare.
 * @param lives     I punti vita attuali del giocatore.
 */
void display_hps(const Position position, const int lives)
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
 * @param position  La posizione in cui stampare.
 * @param score     Il punteggio attuale del giocatore.
 */
void display_score(const Position position, const int score)
{
    #define MAX_SIZE (7 + 8)

    char stringScore[MAX_SIZE];
    snprintf(stringScore, sizeof(stringScore), "Score: %d", score);

    display_string(position, COLOR_CYAN, stringScore, MAX_SIZE);
}

/**
 * Aggiunge una stringa alla lista.
 * @param list      La lista.
 * @param color     Il colore della stringa.
 * @param string    La stringa da aggiungere.
 */
void addStringToList(StringNode **list, int color, char *string) 
{
    StringNode *new = CALLOC(StringNode, 1);
    CRASH_IF_NULL(new)
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

/**
 * Stampa una stringa a schermo.
 * @param position  La posizione in cui stampare.
 * @param color     Il colore della stringa.
 * @param string    La stringa da stampare.
 * @param length    La lunghezza della stringa.
 */
void display_string(const Position position, const int color, const char *string, int length)
{
    eraseFor(position, 1, (short)length);
    
    int pair = alloc_pair(color, getAreaFromY(position.y));

    attron(COLOR_PAIR(pair));
    mvaddstr(position.y, position.x, string);
    attroff(COLOR_PAIR(pair));

    refresh();
}

/**
 * Stampa una stringa a schermo.
 * @param p         La posizione in cui stampare.
 * @param length    La lunghezza della stringa.
 * @param height    L'altezza della stringa.
 * @param list      La lista delle stringhe.
 */
void display_achievements(const Position p, const short length, const short height, StringList list)
{
    if (list.last == NULL) {
        return;
    }

    eraseFor(p, height, length);
    StringNode *last = list.last;
    for (int i = 0, lines = height; i < list.nodes; i++)
    {
        lines -= (int)(last->length / length) + ((last->length % length) > 0);

        if (lines == 0 || lines > 0) {
            if (last->prev) {
                last = last->prev;
            }
            else {
                break;
            }
        }
        else {
            last = last->next;
            break;
        }
    }

    if (last->next == NULL) {
        return;
    }

    const int x = p.x;
    int y = p.y;

    do
    {
        attron(COLOR_PAIR(last->color));
        for (int i = 0, increment = 0; i + increment < last->length; i++)
        {
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

/**
 * Stampa una stringa a schermo.
 * @param height    L'altezza della stringa.
 * @param length    La lunghezza della stringa.
 * @param last      L'ultima stringa.
 * @param map       La mappa di gioco.
 */
void delete_entity_pos(const int height, const int length, const Position last, const MapSkeleton map)
{
    const int old_pair = alloc_pair(COLOR_BLACK, getAreaFromY(GET_MAP_Y(last.y, map)));
    attron(COLOR_PAIR(old_pair));

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < length; j++)
        {
            if (WITHIN_BOUNDARIES(last.x+j, last.y+i, map)) {
                mvaddch(last.y+i, last.x+j, ' ');
            }
        }
    }

    attroff(COLOR_PAIR(old_pair));
}

/**
 * Stampa un'entità a schermo.
 * @param fg    Il colore del testo.
 * @param art   L'arte dell'entità.
 * @param curr  La posizione attuale.
 * @param last  La posizione precedente.
 * @param map   La mappa di gioco.
 */
void display_entity(const int fg, const StringArt art, const Position curr, const Position last, const MapSkeleton map)
{
    if (curr.x == last.x && curr.y == last.y) {
        return;
    }

    const int pair = alloc_pair(fg, getAreaFromY(GET_MAP_Y(curr.y, map)));
    const int length = strlen(art.art[0]);
    const int height = art.length; 

    delete_entity_pos(height, length, last, map);

    attron(COLOR_PAIR(pair));
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < length; j++)
        {
            if (WITHIN_BOUNDARIES(curr.x+j, curr.y+i, map)) {
                mvaddch(curr.y+i, curr.x+j, art.art[i][j]);
            }
        }
    }
    attroff(COLOR_PAIR(pair));
    refresh();
}

/**
 * Costruisce la struttura della mappa.
 * @param map       La mappa di gioco.
 * @param sp        La posizione iniziale.
 * @param width     La larghezza della mappa.
 */
void make_MapSkeleton(MapSkeleton *map, const Position sp, const int width)
{
    int hideouts = GAME_HIDEOUTS;
    while (!(width > hideouts * FROG_WIDTH))
    {
        hideouts--;
    }

    map->hideouts = CALLOC(Position, hideouts + 1);
    CRASH_IF_NULL(map->hideouts)

    int total_left = width - hideouts * FROG_WIDTH;
    int spaces = hideouts + 1;
    int left_per_space = (int) (total_left / spaces);
    int extra_left = total_left % spaces;

    extra_left = extra_left % 2 == 0 ? extra_left : extra_left - 1;
    spaces = spaces % 2 == 0;

    for (int i = 0, spacing = 0; i < hideouts; i++)
    {
        map->hideouts[i].y = sp.y + FROG_HEIGHT; 
        spacing += left_per_space;

        if (extra_left) {
            if (spaces && i + 1 == ((int) (hideouts / 2)) + 1) {
                spacing += extra_left; /* [_0_____0_] */
            }
            else if (i + 1 == ((int) (hideouts / 2)) + 1 || i + 1 == ((int) (hideouts / 2)) + 2) {
                spacing += (int) (extra_left / 2); /* [_0__0__0_] */
            }
        }

        map->hideouts[i].x = sp.x + spacing;
        spacing += FROG_WIDTH;
    }
    map->hideouts[hideouts].y = map->hideouts[hideouts].x = -1;

    map->garden.x = map->sidewalk.x = map->river.x = sp.x;
    map->garden.y = map->hideouts[0].y + FROG_HEIGHT;
    map->river.y = map->garden.y + FROG_HEIGHT * 2;
    map->sidewalk.y = map->river.y + FROG_HEIGHT * GAME_RIVER_LANES;
    map->width = width;
}

/**
 * Disegna la mappa.
 * @param sp    La posizione iniziale.
 * @param width La larghezza della mappa.
 * @param _map  La mappa di gioco.
 * @return      La mappa di gioco.
 */
MapSkeleton display_map(const Position sp, const int width, MapSkeleton _map)
{
    int current_pair = -1;

    for (int i = 0, current_bg = -1, tmp_bg, tmp_pair = 0; i < MAP_HEIGHT; i++)
    {
        tmp_bg = getAreaFromY(i);

        if (tmp_bg != current_bg && (i < 3 || i > 5))
        {
            current_bg = tmp_bg;
            if (current_pair != -1) {
                attroff(COLOR_PAIR(current_pair));
            }
            current_pair = alloc_pair(COLOR_BLACK, current_bg);
            attron(COLOR_PAIR(current_pair));
        }
        else if (tmp_bg != current_bg) {
            tmp_pair = alloc_pair(COLOR_BLACK, tmp_bg);
        }
        
        for (int j = 0, k = 0; j < width; j++)
        {
            if (sp.y + i >= _map.garden.y - 3 && sp.y + i < _map.garden.y - 3 + FROG_HEIGHT) {
                bool ho = false;
                if (_map.hideouts[k].x == 0) {
                    k++;
                }
                
                if (sp.x + j >= _map.hideouts[k].x && sp.x + j < _map.hideouts[k].x + FROG_WIDTH) {
                    ho = true;
                    if (sp.x + j == _map.hideouts[k].x + FROG_WIDTH - 1) {
                        k++;
                    }
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

/**
 * Disegna la schermata di gioco.
 * @param es            La lista delle entità.
 * @param map           La mappa di gioco.
 * @param mainClock     L'orologio principale.
 * @param secClock      L'orologio secondario.
 * @param achievements  La lista degli obiettivi.
 * @param score         Il punteggio.
 * @param lives         I punti vita.
 * @param drawAll       Se disegnare tutto.
 */
void draw(  struct entities_list *es,
            MapSkeleton *map,
            Clock *mainClock, Clock *secClock,
            int score,
            int lives,
            bool drawAll)
{
    Position POSITION_MAP = {
        MAP_START_X,
        MAP_START_Y
    };

    Position POSITION_LIVES = {
        45 + MAP_START_X,
        3
    };

    Position POSITION_SCORE = {
        MAP_START_X + 2,
        3
    };

    Position POSITION_MAIN_CLOCK = {
        75 + MAP_START_X,
        2
    };

    Position POSITION_SEC_CLOCK = {
            75 + MAP_START_X,
            5
    };

    if (drawAll) {
        *map = display_map(POSITION_MAP, MAP_WIDTH, *map);
    }

    display_hps(POSITION_LIVES, lives);
    display_score(POSITION_SCORE, score);

    display_clock(POSITION_MAIN_CLOCK, mainClock->current, mainClock->starting);
    display_clock(POSITION_SEC_CLOCK, secClock->current, secClock->starting);

    for (int i = 0; i < 5; i++)
    {
        struct entities_list *ec = es;
        while (ec != NULL)
        {
            Entity *target = ec->e;
            if (getPriorityByEntityType(target->type) == i) {
                display_entity(getEntityColor(target->trueType), getArtOfEntity(target), target->current, target->last, *map);
            }
            ec = ec->next;
        }
    }
}

/**
 * Stampa una stringa di debug.
 * @param y             La coordinata Y.
 * @param __format      Il formato della stringa.
 * @param stringLength  La lunghezza della stringa.
 * @param ...           Gli argomenti della stringa.
 */
void display_debug_string(int y, const char *__restrict __format, int stringLength, ...)
{
    static int x = 140;

    char *string = MALLOC(char, stringLength);
    va_list args;
    va_start(args, stringLength);
    vsnprintf(string, stringLength, __format, args);
    va_end(args);

    display_string(getPosition(x, y), COLOR_CYAN, string, stringLength);
    free(string);
}
