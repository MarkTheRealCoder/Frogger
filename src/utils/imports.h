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

#define TEST_MODE true
#define DEBUG_MODE false

// Stampa un messaggio se la modalità di debug è attiva.
#define DEBUG(fmt, ...)             \
    if (DEBUG_MODE)                 \
    {                               \
        printf(fmt, ##__VA_ARGS__); \
    }
