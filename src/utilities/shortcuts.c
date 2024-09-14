#include "shortcuts.h"

/**
 * Genera un numero casuale in un determinato range X->Y (inclusi).
 * @param min   Estremo minore.
 * @param max   Estremo maggiore.
 * @return      Il numero casuale generato.
 */
int gen_num(const int min, const int max)
{
    return min + rand() % (max - min + 1);
}

/**
 * Sceglie un valore tra quelli passati come argomenti.
 * @param count Il numero di argomenti.
 * @param ...   Gli argomenti.
 * @return      Il valore scelto.
 */
int choose_between(const int count, ...)
{
    va_list args;
    va_start(args, count);

    int random_index = gen_num(0, count - 1);
    int result;
    
    for (int i = 0; i < count; i++) 
    {
        int current = va_arg(args, int);
        
        if (i == random_index) 
        {
            result = current;
        }
    }

    va_end(args);

    return result;
}

bool choose_between_bool()
{
    return gen_num(0, 1);
}

/**
 * Esegue una sleep per un determinato periodo di tempo.
 * @param quantity      La quantità di tempo.
 * @param timeFrame     L'unità di misura del tempo.
 */
void sleepy(const int quantity, const TimeFrame timeFrame)
{
    int multiplier = timeFrame == TIMEFRAME_MICROS ? 1 : timeFrame == TIMEFRAME_MILLIS ? 1000 : 1000000;
    usleep(multiplier * quantity);
}

/**
 * Controlla se due stringhe sono uguali.
 * @param expected      La stringa attesa.
 * @param toCompare     La stringa da confrontare.
 * @return              Se le due stringhe sono uguali.
 */
bool str_eq(char *expected, char *toCompare)
{
    return strcmp(expected, toCompare) == 0;
}

/**
 * Concatena due o più stringhe in base al numero di stringhe specificato e alle stringhe specificate.
 * @param n     Il numero di stringhe da concatenare.
 * @param ...   Le stringhe in ordine di comparizione da concatenare.
 * @return      L'unione di tutte le stringhe.
 */
char *concat(const int n, ...) 
{
    char *output = NULL;

    va_list args;
    va_start(args, n);
    
    int count = 0;
    
    for (int i = 0; i < n; i++) 
    {
        char *str = (char*) va_arg(args, char *);
        int tlen = strlen(str);

        if (!output)
        {
            output = CALLOC(char, tlen + 1);
        }
        else 
        {
            output = REALLOC(char, output, tlen + strlen(output) + 1);
        }

        CRASH_IF_NULL(output)

        for (int j = 0; j < tlen; count++, j++) 
        {
            output[count] = str[j];
        }
        output[count] = 0;
    }

    va_end(args);
    return output;
}

/**
 * Prende la dimensione attuale del terminale.
 * @return  Un array di due interi [x, y] contenente le dimensioni del terminale.
 */
int *get_screen_size()
{
    static int size[2] = {0, 0};

    getmaxyx(stdscr, size[0], size[1]);

    return size;
}

/**
 * Restituisce la posizione centrata sull'asse X.
 * @param width     La larghezza dell'elemento.
 * @return          La posizione centrata sull'asse X.
 */
int getCenteredX(const int width)
{
    const int x = get_screen_size()[1];
    return (x / 2) - (width / 2);
}

/**
 * Restituisce la posizione centrata sull'asse Y.
 * @param height    La larghezza dell'elemento.
 * @return          La posizione centrata sull'asse Y.
 */
int getCenteredY(const int height)
{
    const int y = get_screen_size()[0];
    return (y / 2) - (height / 2);
}

/**
 * Restituisce l'icona corrispondente all'entità'.
 * @param entity    L'entità'.
 * @return          L'icona corrispondente all'entità'.
 */ 
StringArt getArtOfEntity(const Entity *entity)
{
    StringArt result = { };

    result.length = ART_OF_LENGTH_3;

    switch (entity->type)
    {
        case ENTITY_TYPE__CROC:
            result.art = _CROC_X3_PLAY_ART;
            break;
        case ENTITY_TYPE__FROG:
            result.art = _FROG_PLAY_ART;
            break;
        case ENTITY_TYPE__PLANT:
            result.art = _PLANT_PLAY_ART;
            break;
        case ENTITY_TYPE__PROJECTILE:
            result.art = _PROJECTILE_PLAY_ART;
            result.length = ART_OF_LENGTH_1;
        default:
            break;
    }

    return result;
}

/**
 * Restituisce l'icona corrispondente al tipo di icona'.
 * @param art       Il contenuto dell'icona.
 * @param length    La lunghezza dell'icona.
 * @return          L'icona corrispondente all'entità'.
 */
StringArt getArtOfThing(const enum AVAILABLE_ARTS artid, char **art, const int length)
{
    StringArt result = { };

    switch (artid)
    {
        case ART_BIG_FROG:
            result.art = _FROG_ART;
            result.length = _FROG_ART_LENGTH;
            break;
        case ART_TWO_FROGS:
            result.art = _FROGGER_PAUSE_ART;
            result.length = _FROGGER_PAUSE_LENGTH;
            break;
        case ART_PAUSE_LOGO:
            result.art = _FROGGER_PAUSE_LOGO,
            result.length = _FROGGER_PAUSE_LOGO_LENGTH;
            break;
        case ART_MAIN_LOGO:
            result.art = _FROGGER_LOGO,
            result.length = _FROGGER_LOGO_LENGTH;
            break;
        case ART_LOST_LOGO:
            result.art = _FROGGER_LOST_LOGO,
            result.length = _FROGGER_LOST_LOGO_LENGTH;
            break;
        case ART_WIN_LOGO:
            result.art = _FROGGER_WIN_LOGO,
            result.length = _FROGGER_WIN_LOGO_LENGTH;
            break;
        default:
            result.art = art;
            result.length = length;
            break;
    }

    return result;
}

/**
 * Controlla se l'azione è di movimento.
 * @param action    L'azione da controllare.
 * @return          Se l'azione è di movimento o meno.
 */
bool isActionMovement(const Action action)
{
    return action >= ACTION_NORTH && action <= ACTION_WEST;
}

int getInnerMiddleWithOffset(int width, int divTimes, int indexToPick, int entityWidth)
{
    int toDivide = divTimes * 2;

    if (indexToPick >= toDivide)
    {
        return -1;
    }

    int segment = width / toDivide;

    int point = segment * indexToPick;
    point -= entityWidth / 2;

    return point;
}

Position getPositionWithInnerMiddleX(int width, int height, int divTimes, int indexToPick, int entityWidth)
{
    int x = MAP_START_X + getInnerMiddleWithOffset(width, divTimes, indexToPick, entityWidth);
    int y = height;

    return getPosition(x, y);
}

bool areHideoutsClosed(const MapSkeleton *map)
{
    int counter = 0;

    for (int i = 0; map->hideouts[i].x != -1; ++i)
    {
        if (map->hideouts[i].x != 0 && map->hideouts[i].y != 0)
        {
            counter = counter | (1 << i);
        }
    }

    return counter;
}

int isEntityPositionHideoutValid(const Entity *entity, const MapSkeleton *map)
{
    if (entity->type != ENTITY_TYPE__FROG || entity->current.y != (map->garden.y - FROG_HEIGHT))
    {
        return true;
    }

    for (int i = 0; map->hideouts[i].x != -1; i++)
    {
        if (entity->current.x == map->hideouts[i].x && entity->current.y == map->hideouts[i].y)
        {
            return i + 2;
        }
    }

    return false;
}

Component getDefaultClockComponent(const enum ClockType clockType)
{
    #define MAIN_VALUE 120
    #define SECONDARY_VALUE 2

    int value = clockType == CLOCK_MAIN ? MAIN_VALUE : SECONDARY_VALUE;

    return (Component) {
        .type = COMPONENT_CLOCK,
        .component = create_clock(value, clockType)
    };
}

Component getDefaultEntitiesComponent()
{
    return (Component) {
        .type = COMPONENT_ENTITIES,
        .component = create_entities_group()
    };
}

char *numToString(int num) {
    char *numb = (char*) calloc(11, sizeof(char));
    numb[0] = '0';
    int dim = 0;
    while (num != 0) {
        for (int i = dim; dim != 0 && i > -1; i--) {
            numb[i] = numb[i - 1];
        }
        numb[0] = (char)((num % 10) + 48);
        num /= 10;
        dim++;
    }
    numb = (char*) realloc(numb, sizeof(char)*(strlen(numb) + 1));
    return numb;
}

// todo funzione temporizzata