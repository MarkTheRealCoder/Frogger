#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <stdatomic.h>
#include <semaphore.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>    // For O_* constants
#include <sys/mman.h> // For shm_open, mmap, PROT_* constants
#include <ncurses.h>
#include <locale.h>

#ifndef IMPORT
#define IMPORT

#pragma once

#ifndef GAME_ADDONS
#define GAME_ADDONS
#include "../utilities/addons.h"
#endif

#ifndef GAME_THREADS
#define GAME_THREADS
#include "../concurrency/threads.h"
#endif

#ifndef GAME_PROCESSES
#define GAME_PROCESSES
#include "../concurrency/processes.h"
#endif

#ifndef GAME_DRAWING
#define GAME_DRAWING
#include "drawing.h"
#endif

#ifndef GAME_ENTITIES
#define GAME_ENTITIES
#include "entities.h"
#endif

#ifndef GAME_SHORTCUTS
#define GAME_SHORTCUTS
#include "../utilities/shortcuts.h"
#endif

#ifndef GAME_COMMON
#define GAME_COMMON
#include "../utilities/common.h"
#endif

#endif

static bool GLOBAL_SCREEN_INVALID_SIZE = false;

#define TEST_MODE true
#define DEBUG_MODE false

// Stampa un messaggio se la modalità di debug è attiva.
#define DEBUG(fmt, ...)             \
    if (DEBUG_MODE)                 \
    {                               \
        printf(fmt, ##__VA_ARGS__); \
    }
