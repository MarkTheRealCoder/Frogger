#include "addons.h"

/**
 * Esegue il parsing degli argomenti passati al programma.
 * @param argc  Il numero di argomenti.
 * @param argv  Gli argomenti.
 * @return      La struttura delle opzioni passate.
 */
struct program_args addons_parse_args(int argc, char *argv[])
{
    struct program_args parsed_arguments = { };
    char *current_arg;

    for (int i = 1; i < argc; i++) 
    {
        current_arg = argv[i];

        if (str_eq("-h", current_arg) || str_eq("--help", current_arg)) {
            parsed_arguments.help = true;
        }
    }

    return parsed_arguments;
}

/**
 * Mostra l'help dei comandi.
 */
void addons_args_help()
{
    printf("Usage: frogger [options]\n ");
    printf("Options:\n");
    printf("\t-h, --help\t\tShows this help message\n");
}
