#ifndef FROGGER_GLOBALS_H
#define FROGGER_GLOBALS_H

static bool GLOBAL_SCREEN_INVALID_SIZE = false;


static pthread_mutex_t GLOBAL_MANCHE_ENDED_MUTEX = PTHREAD_MUTEX_INITIALIZER;
static atomic_bool GLOBAL_GAME_PAUSED = false;
static atomic_bool GLOBAL_GAME_ENDED = false;


static Position GLOBAL_MAP_POSITION = { };
static Position GLOBAL_FROG_POSITION = { };

static Position GLOBAL_SCORE_POSITION = { };
static Position GLOBAL_LIVES_POSITION = { };
static Position GLOBAL_TIMER_POSITION = { };

static Position GLOBAL_LOGS_POSITION = { };
static Position GLOBAL_ACHIEVEMENT_POSITION = { };

void setScreenValidity(bool value);
bool isScreenValid();

void setGamePaused(bool value);
bool isGamePaused();

void setGameEnded(bool value);
bool isGameEnded();

void lockMancheEndedMutex();
void unlockMancheEndedMutex();

#endif //FROGGER_GLOBALS_H
