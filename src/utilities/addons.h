#ifndef FROGGER_ADDONS_H
#define FROGGER_ADDONS_H

#include "../commons/imports.h"
#include "../commons/structures.h"

/**
 * Program arguments.
 */

struct program_args 
{
    bool help;
};

struct program_args addons_parse_args(int argc, char *argv[]);
void addons_args_help();

#endif // !FROGGER_ADDONS_H
