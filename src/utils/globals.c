#include "shortcuts.h"
#include "globals.h"

/**
 * Imposta la validità dello schermo.
 * @param value Il valore da impostare.
 */
void setScreenValidity(bool value)
{
    GLOBAL_SCREEN_INVALID_SIZE = value;
}

/**
 * Controlla se lo schermo è valido.
 * @return  Se lo schermo è valido.
 */
bool isScreenValid()
{
    return !GLOBAL_SCREEN_INVALID_SIZE;
}

/**
 * Imposta se il gioco è in pausa.
 * @param value Il valore da impostare.
 */
void setGamePaused(bool value)
{
    atomic_store(&GLOBAL_GAME_PAUSED, value);
}

/**
 * Controlla se il gioco è in pausa.
 * @return  Se il gioco è in pausa.
 */
bool isGamePaused()
{
    return atomic_load(&GLOBAL_GAME_PAUSED);
}

/**
 * Imposta se il gioco è terminato.
 * @param value Il valore da impostare.
 */
void setGameEnded(bool value)
{
    atomic_store(&GLOBAL_GAME_ENDED, value);
}

/**
 * Controlla se il gioco è terminato.
 * @return  Se il gioco è terminato.
 */
bool isGameEnded()
{
    return atomic_load(&GLOBAL_GAME_ENDED);
}

/**
 * Blocca il mutex della manche terminata.
 */
void lockMancheEndedMutex()
{
    pthread_mutex_lock(&GLOBAL_MANCHE_ENDED_MUTEX);
}

/**
 * Sblocca il mutex della manche terminata.
 */
void unlockMancheEndedMutex()
{
    pthread_mutex_unlock(&GLOBAL_MANCHE_ENDED_MUTEX);
}

void initPositions()
{
    GLOBAL_MAP_POSITION = (Position) {
        .x = 0,
        .y = 0
    };

    GLOBAL_FROG_POSITION = (Position) {
        .x = 0,
        .y = 0
    };

    GLOBAL_SCORE_POSITION = (Position) {
        .x = 0,
        .y = 0
    };

    GLOBAL_LIVES_POSITION = (Position) {
        .x = 0,
        .y = 0
    };

    GLOBAL_TIMER_POSITION = (Position) {
        .x = 0,
        .y = 0
    };

    GLOBAL_LOGS_POSITION = (Position) {
        .x = 0,
        .y = 0
    };

    GLOBAL_ACHIEVEMENT_POSITION = (Position) {
        .x = 0,
        .y = 0
    };
}