#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <stdatomic.h>
#include <semaphore.h>

#define TEST_MODE true
#define DEBUG_MODE true

// Stampa un messaggio se la modalità di debug è attiva.
#define DEBUG(fmt, ...)             \
    if (DEBUG_MODE)                 \
    {                               \
        printf(fmt, ##__VA_ARGS__); \
    }
