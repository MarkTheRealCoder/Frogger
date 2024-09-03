#include "shortcuts.h"

/**
 * Genera un numero casuale in un determinato range X->Y (inclusi).
 * @param min   Estremo minore.
 * @param max   Estremo maggiore.
 * @return      Il numero casuale generato.
 */
int gen_num(int min, int max)
{
    return min + rand() % (max - min + 1);
}

/**
 * Sceglie un valore tra quelli passati come argomenti.
 * @param count Il numero di argomenti.
 * @param ...   Gli argomenti.
 * @return      Il valore scelto.
 */
int choose_between(int count, ...) 
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

/**
 * Esegue una sleep per un determinato periodo di tempo.
 * @param quantity      La quantità di tempo.
 * @param timeFrame     L'unità di misura del tempo.
 */
void sleepy(int quantity, TimeFrame timeFrame)
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
 * Restituisce la stringa corrispondente alla direzione.
 * @param direction La direzione.
 * @return          La stringa corrispondente alla direzione.
 */
char *str_direction(Action direction)
{
    #define DIRECTION_COUNT 5

    if (direction > DIRECTION_COUNT - 1)
    {
        return "INVALID";
    }

    static char *directions[] = {
        "NORTH",
        "SOUTH",
        "EAST",
        "WEST",
        "STILL"
    };

    return directions[direction];
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
    int rows, cols;

    getmaxyx(stdscr, rows, cols); 

    return (int[]) { rows, cols };
}

/**
 * Restituisce la posizione centrata sull'asse X.
 * @param height    L'altezza dell'elemento.
 * @return          La posizione centrata sull'asse X.
 */
int getCenteredX(const int height)
{
    const int x = get_screen_size()[1];
    return (x / 2) - (height / 2);
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
StringArt getArtOfThing(enum AVAILABLE_ARTS artid, char **art, const int length)
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

