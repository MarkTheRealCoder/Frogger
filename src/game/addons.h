#ifndef FROGGER_ADDONS_H
#define FROGGER_ADDONS_H

#include "../utils/shortcuts.h"

/*
 * Program arguments.
 */

struct program_args 
{
    bool help;
    bool quit_on_win;
    bool save_game_stats;
};

struct program_args parse_program_args(int argc, char *argv[]);
void program_args_print_help();
void program_args_save_stats();  

#endif // !FROGGER_ADDONS_H
