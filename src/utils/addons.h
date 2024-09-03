#ifndef FROGGER_ADDONS_H
#define FROGGER_ADDONS_H

#include "imports.h"

/**
 * Program arguments.
 */

struct program_args 
{
    bool help;
    bool quit_on_win;
    bool save_game_stats;
    bool save_replay;
    bool replay_view;
};

struct program_args addons_parse_args(int argc, char *argv[]);
void addons_args_help();
void addons_args_save_stats(struct game_threads *game_threads);

#endif // !FROGGER_ADDONS_H
