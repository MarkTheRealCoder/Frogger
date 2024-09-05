#ifndef THREADS_H
#define THREADS_H

#include "../commons/imports.h"
#include "../commons/structures.h"
#include "../commons/drawing.h"
#include "./shared/shared.h"

typedef struct {
    ProductionRules rules;
    int *buffer;
} ThreadCarriage;


static int COMMUNICATIONS = 0;
static sem_t COMMUNICATION_SEMAPHORE;
static sem_t POLLING_WRITING;
static sem_t POLLING_READING;


/*
                     ,.
                   (\(\)
   ,_              ;  o >
    {`-.          /  (_)
    `={\`-._____/`   |
     `-{ /    -=`\   |
   .="`={  -= = _/   /`"-.
  (M==M=M==M=M==M==M==M==M)
   \=N=N==N=N==N=N==N=NN=/
    \M==M=M==M=M==M===M=/
     \N=N==N=N==N=NN=N=/
  jgs \M==M==M=M==M==M/
       `-------------'
          "BAK BAK"
 */
PollingResult thread_polling_routine(int buffer[MAX_CONCURRENCY], GameSkeleton *game);
void thread_main(GameSkeleton *game, struct entities_list **list);

void init_semaphores();
void close_semaphores();

#endif //THREADS_H
