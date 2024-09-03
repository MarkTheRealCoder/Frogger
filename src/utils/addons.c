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

        if (str_eq("-h", current_arg) || str_eq("--help", current_arg))
        {
            parsed_arguments.help = true;
        }

        if (str_eq("-q", current_arg) || str_eq("--quit_after_win", current_arg))
        {
            parsed_arguments.quit_on_win = true;
        }

        if (str_eq("-s", current_arg) || str_eq("--save_game_stats", current_arg))
        {
            parsed_arguments.save_game_stats = true;
        }

        if (str_eq("-rs", current_arg) || str_eq("--replay_save", current_arg))
        {
            parsed_arguments.save_replay = true;
        }

        if (str_eq("-rv", current_arg) || str_eq("--replay_view", current_arg))
        {
            parsed_arguments.replay_view = true;
        }
    }

    return parsed_arguments;
}

/**
 * Mostra l'help dei comandi.
 */
void addons_args_help()
{
    printf("Usage: frogger [options]\n");
    printf("Options:\n");
    printf("\t-h, --help\t\tShows this help message\n");
    printf("\t-q, --quit_after_win\tQuits the program after game is won.\n");
    printf("\t-s, --save_game_stats\tSaves the last game's statistics to file.\n");
    printf("\t-rs, --replay_save\tSaves the last game replay to a file.");
    printf("\t-rv, --replay_view\tViews the saved replays.");
}

/**
 * Salva le statistiche di gioco su file.
 * @param game  La struttura del gioco.
 */
void addons_args_save_stats(struct game_threads *game)
{
    FILE *file;

    char date[24];
    char file_name[34];

    time_t current_time = time(NULL);

    strftime(date, 24, "%Y-%m-%d+%H-%M-%S", localtime(&current_time));
    snprintf(file_name, sizeof(file_name), "frogger_%s.txt", date);

    file = fopen(file_name, "w");
    
    fprintf(file, "Statistiche della partita di Frogger del `%s`\n\n", date);

    fprintf(file, "DETTAGLI PARTITA\n");
    fprintf(file, "\t* ...\n");

    fclose(file); 
}
