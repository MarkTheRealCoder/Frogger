#include "addons.h"

struct program_args parse_program_args(int argc, char *argv[])
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
    }

    return parsed_arguments;
}

void program_args_print_help()
{
    printf("Usage: frogger [options]\n");
    printf("Options:\n");
    printf("\t-h, --help\t\tShows this help message\n");
    printf("\t-q, --quit_after_win\tQuits the program after game is won.\n");
    printf("\t-s, --save_game_stats\tSaves the last game's statistics to file.\n");
}

void program_args_save_stats()
{
    FILE *file;
    time_t current_time = time(NULL);
    char file_name[34];
    
    strftime(file_name, 34, "frogger_%Y-%m-%d+%H-%M-%S.txt", localtime(&current_time));

    file = fopen(file_name, "w");

    // todo: salvare ogni statistica di gioco
    fprintf(file, "statistica: XX\n");
    
    fclose(file); 
}

