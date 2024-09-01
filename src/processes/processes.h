#ifndef PROCESSES_H
#define PROCESSES_H

#include "../utils/imports.h"
#include "../graphics/drawing.h"
#include "../utils/shortcuts.h"

#define PROCESS_NUM 24

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define HANDLE_ERROR(func)                              \
if (func <= 0)                                          \
{                                                       \
    perror("[Error Handler] Something went wrong\n");   \
    raise(SIGUSR1);                                     \
}

#define READ 0
#define WRITE 1
static struct timeval tv = {.tv_sec = 0, .tv_usec = 50};
#define ACCEPTABLE_WAITING_TIME &tv

#define CLOSE_READ(_pipe)   \
{close(_pipe.accesses[READ]);}
#define CLOSE_WRITE(_pipe)  \
{close(_pipe.accesses[WRITE]);}

#define PIPE_NAME 32
#define FAILED_PIPE (pipe_t){.name=NULL, .accesses={-1,-1}}
#define PAS 5 // pipe array size
#define MIN_PAS 2

#define PIPE_SIZE 2

#define SERVICE_NAME "/service_comms"
#define SERVICE_SIZE 1024
#define MATCH_ID(id, message) ((message) >> 4) == 0 || ((message) >> 4) & (id)
#define AVAILABLE_DYNPID(result, ids)   \
{                                       \
    int copy = ids, count = 0;          \
                                        \
    while (copy & 1)                    \
    {                                   \
        count++;                        \
        copy = copy >> 1;               \
    }                                   \
                                        \
    result = 1 << (count);              \
    ids = ids | result;                 \
}


typedef enum {
    STATUS_RUNNING, 
    STATUS_IDLE, 
    STATUS_ENDED
} Status;

typedef enum {
    MESSAGE_RUN=3, 
    MESSAGE_HALT=2,
    MESSAGE_STOP=1, 
    MESSAGE_NONE=0
} pMessages;

typedef struct {
    int accesses[PIPE_SIZE];
    char *name;
} pipe_t;

typedef struct {
    pipe_t * pipes;
    unsigned int size;
} Pipes;

typedef struct {
    pid_t pid;
    int dynamic_pid;
    Status status;
} Process;


#endif
