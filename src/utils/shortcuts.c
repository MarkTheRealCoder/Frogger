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
 * Concatena due o più stringhe in base al numero di stringhe specificato e alle stringhe specificate.
 * @param n     Il numero di stringhe da concatenare.
 * @param ...   Le stringhe in ordine di comparizione da concatenare.
 * @return      L'unione di tutte le stringhe.
 */

char *concat(const int n, ...) {
    char *output = CALLOC(char, 250);
    va_list args;
    va_start(args, n);
    int count = 0;
    for (int i = 0; i < n; i++) {
        char *str = (char*) va_arg(args, char *);
        int tlen = strlen(str);
        for (int j = 0; j < tlen; count++, j++) {
            output[count] = str[j];
        }
    }
    va_end(args);
    output[count+1] = 0;
    output = REALLOC(char, output, strlen(output+1));
    return output;
}
