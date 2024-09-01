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
char *str_direction(Direction direction)
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
 * @param width La larghezza dell'elemento.
 * @return      La posizione centrata sull'asse X.
 */
int getCenteredX(int width)
{
    int x = get_screen_size()[1];
    return (x / 2) - (width / 2);
}

/**
 * Restituisce la posizione centrata sull'asse Y.
 * @param height    L'altezza dell'elemento.
 * @return          La posizione centrata sull'asse Y.
 */
int getCenteredY(int height)
{
    int y = get_screen_size()[0];
    return (y / 2) - (height / 2);
}

/**
 * Restituisce l'icona corrispondente all'entità'.
 * @param entity    L'entità'.
 * @return          L'icona corrispondente all'entità'.
 */ 
StringArt getArt(struct entity *entity)
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
        default:
            break;
    }

    return result;
}
