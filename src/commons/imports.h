#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <ncurses.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>

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

#define TEST_MODE true
#define DEBUG_MODE false

// Stampa un messaggio se la modalità di debug è attiva.
#define DEBUG(fmt, ...)             \
    if (DEBUG_MODE)                 \
    {                               \
        printf(fmt, ##__VA_ARGS__); \
    }
