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
 * Conta il numero di cifre di un numero.
 * @param value Il numero.
 * @return      Il numero di cifre del numero.
 */
int count_digits(int value)
{
    int digits = 0;

    while (value > 0)
    {
        value /= 10;
        ++digits;
    }

    return digits;
}

/**
 * Divide due numeri se possibile altrimenti ritorna il dividendo.
 * @param dividend  Il dividendo.
 * @param divisor   Il divisore.
 * @return          La divisione tra i due numeri, se possibile.
 */
int divide_if_possible(int dividend, int divisor)
{
    if (divisor == 0 || dividend == 0)
    {
        return 0;
    }

    return dividend >= divisor ? dividend / divisor : dividend;
}

/**
 * Calcola la percentuale.
 * @param dividend  Il dividendo.
 * @param divisor   Il divisore.
 */
double calculate_percentage(int dividend, int divisor)
{
    if (divisor == 0 || dividend == 0)
    {
        return 0;
    }

    return ((double) dividend / divisor) * 100;
}

/*
 * Esegue una sleep per un determinato periodo di tempo.
 * @param quantity      La quantità di tempo.
 * @param timeFrame     L'unità di misura del tempo.
 */
void sleepy(int quantity, TimeFrame timeFrame)
{
    int multiplier = timeFrame == TIMEFRAME_MICROS ? 1 : timeFrame == TIMEFRAME_MILLIS ? 1000 : 1000000;
    usleep(multiplier * quantity);
}

/*
 * Controlla se due stringhe sono uguali.
 * @param expected      La stringa attesa.
 * @param toCompare     La stringa da confrontare.
 * @return              Se le due stringhe sono uguali.
 */
bool str_eq(char *expected, char *toCompare)
{
    return strcmp(expected, toCompare) == 0;
}

/*
 * Restituisce la stringa corrispondente al tipo di pacchetto.
 * @param packetType    Il tipo di pacchetto.
 * @return              La stringa corrispondente al tipo di pacchetto.
 */
char *str_packet_type(PacketType packetType)
{
    static char *packetTypes[] = {
        "VOID",
        "INT",
        "TIMER",
        "GAMETHREADS",
        "ENTITYMOVE"
    };

    return packetTypes[packetType];
}

/**
 * Restituisce la stringa corrispondente alla direzione.
 * @param direction La direzione.
 * @return          La stringa corrispondente alla direzione.
 */
char *str_direction(Direction direction)
{
    static char *directions[] = {
        "NORTH",
        "SOUTH",
        "EAST",
        "WEST",
    };

    return directions[direction];
}
