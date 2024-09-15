#include "../drawing.h"

void print_art(StringArt art, int *cuy, int max)
{
    int pair = alloc_pair(COLORCODES_FROG_ART, COLOR_BLACK);

    attron(COLOR_PAIR(pair) | A_BOLD);
    for (size_t i = 0; i < art.length; i++)
    {
        center_string(art.art[i], max, *cuy);
        (*cuy)++;
    }
    attroff(COLOR_PAIR(pair) | A_BOLD);
}

/**
 * Stampa il logo del gioco.
 * @param color Il colore da utilizzare.
 * @param max La larghezza della finestra
 */
void print_logo(StringArt logo, int pair, int max, int *cuy)
{
    attron(COLOR_PAIR(pair) | A_BOLD);
    for (size_t i = 0; i < logo.length; i++)
    {
        center_string(logo.art[i], max, *cuy);
        (*cuy)++;
    }
    attroff(COLOR_PAIR(pair) | A_BOLD);
    refresh();
}

/**
 * Stampa le scelte del menu'.
 * @param choices           Le scelte.
 * @param choices_num       Il numero di scelte.
 * @param current_choice    La scelta corrente.
 * @param max               La larghezza della finestra.
 * @param cuy               La y corrente.
 */
void print_choices(char **choices, int choices_num, int current_choice, int max, int *cuy)
{
    int standard    = alloc_pair(COLORCODES_FROG_ART_LOGO, COLOR_BLACK);
    int quit        = alloc_pair(COLORCODES_FROG_ART_LOGO_QUIT, COLOR_BLACK);
    int selected    = alloc_pair(COLORCODES_FROG_ART_SELECTED, COLOR_BLACK);
    int starting_choice = 0;

    for (size_t i = starting_choice; i < choices_num; i++)
    {
        (*cuy)++;

        if (current_choice == i) {
            char *tmp = concat(3, "> ", choices[i], " <\n");
            center_string_colored(tmp, selected, max, *cuy-1);
            free(tmp);
            continue;
        }
        center_string_colored(choices[i], i == (choices_num - 1) ? quit : standard, max, *cuy-1);
    }

    refresh();
}

/**
 * Listener per il menu.
 * @param choice        La scelta corrente.
 * @param choices_num   Il numero di scelte.
 * @return              Se l'utente ha premuto invio.
 */
bool menu_listener(int *choice, int choices_num)
{
    int c = wgetch(stdscr);
    bool not_exit = true;

    switch (c)
    {
        case KEY_DOWN:
        case 's':
            if (*choice < choices_num - 1) {
                (*choice)++;
            }
            break;
        case KEY_UP:
        case 'w':
            if (*choice) {
                (*choice)--;
            }
            break;
        case KEY_ENTER:
        case '\r':
        case '\n':
            not_exit = false;
            break;
        default:
            break;
    }

    return not_exit;
}

/**
 * Propone un menu' generico all'utente.
 * @param screen    La struttura dello schermo.
 * @param choices   Le scelte disponibili.
 * @param logo      Il logo del gioco.
 * @param art       L'arte da stampare.
 * @return          La scelta effettuata.
 */
int generic_menu(const Screen screen, StringArt choices, StringArt logo, StringArt art)
{
    // Pulisce lo schermo da ogni elemento disegnato in precedenza
    clear_screen();

    // Ricerco e/o creo le coppie (BG/FG) per i colori da usare per il menù
    int standard = alloc_pair(COLORCODES_FROG_ART_LOGO, COLOR_BLACK);
    int quit = alloc_pair(COLORCODES_FROG_ART_LOGO_QUIT, COLOR_BLACK);

    // Scelta iniziale del menù (default: 0 prima voce disponibile)
    int choice = 0;
    // la y corrente da cui inizia la stampa.
    int cuy = 1;

    // Stampa dell'immagine del menù (le StringArt sono molto flessibili e per arte possono contenere un insieme di stringhe generico)
    print_art(art, &cuy, screen.x);
    // Incremento la y di una riga per spostare ulteriormente la stampa in basso e lasciare uno spazio dall'art.
    cuy++;

    // Creo una variabile d'appoggio per comodità per resettare più agevolmente la y a ogni ciclo.
    int logo_cuy = cuy;

    do 
    {
        // Resetto preventivamente la y a ogni inizio di ciclo.
        cuy = logo_cuy;
        // Stampo il logo del gioco selezionando il colore in base all'opzione scelta dall'utente: di default l'opzione di uscita è l'ultima.
        print_logo(logo, choice == choices.length - 1 ? quit : standard, screen.x, &cuy);

        // Incremento la y di 4 righe per dare spazio alla sezione delle scelta allontanandola dal logo.
        cuy += 4;
        // Stampo le scelte del menù disponibili.
        print_choices(choices.art, choices.length, choice, screen.x, &cuy);
        // Esco dal ciclo solo una volta che l'utente ha premuto invio, in caso contrario aggiorno solo l'indice della scelta.
    } while (menu_listener(&choice, choices.length));

    // Pulisco nuovamente lo schermo da ogni elemento disegnato in precedenza
    clear_screen();

    // Restituisco la scelta effettuata.
    return choice;
}

/**
 * Propone il menu principale.
 * @param screen    La struttura dello schermo.
 * @return          La scelta effettuata.
 */
int main_menu(const Screen screen)
{
    #define MENU_MAIN_LEN 2
    static char *choices[MENU_MAIN_LEN] = {
        "Start new game",
        "Quit"
    };

    return generic_menu(screen, getArtOfThing(ART_UNKNOWN , choices, MENU_MAIN_LEN),
                                getArtOfThing(ART_MAIN_LOGO, NULL, 0),
                                getArtOfThing(ART_BIG_FROG, NULL, 0));
}

/**
 * Propone il menu di pausa.
 * @param screen    La struttura dello schermo.
 * @return          La scelta effettuata.
 */
int pause_menu(const Screen screen)
{
    #define MENU_PAUSE_LEN 2
    static char *choices[MENU_PAUSE_LEN] = {
        "Resume",
        "Quit"
    };

    return generic_menu(screen, getArtOfThing(ART_UNKNOWN, choices, MENU_PAUSE_LEN),
                                getArtOfThing(ART_PAUSE_LOGO, NULL, 0),
                                getArtOfThing(ART_TWO_FROGS, NULL, 0));
}

/**
 * Propone il menu di scelta tra thread e processi.
 * @param screen    La struttura dello schermo.
 * @return          La scelta effettuata.
 */
int thread_or_processes_menu(const Screen screen)
{
    #define MENU_THREAD_OR_PROCESSES_LEN 3
    static char *choices[MENU_THREAD_OR_PROCESSES_LEN] = {
        "Threads version",
        "Processes version",
        "Quit"
    };

    return generic_menu(screen, getArtOfThing(ART_UNKNOWN, choices, MENU_THREAD_OR_PROCESSES_LEN),
                                getArtOfThing(ART_MAIN_LOGO, NULL, 0),
                                getArtOfThing(ART_BIG_FROG, NULL, 0));
}

/**
 * Propone il menu di fine partita.
 * @param screen    La struttura dello schermo.
 * @param state     Lo stato della partita.
 * @param score     Il punteggio ottenuto.
 * @return          La scelta effettuata.
 */
int game_over_menu(const Screen screen, const enum PS state, const int score)
{
    #define MENU_GAME_OVER_LEN 2

    static char *choices[MENU_GAME_OVER_LEN] = {
            "Play Again",
            "Quit"
    };

    static char *partOfMessage = "You scored: ";

    enum AVAILABLE_ARTS art = state == PS_LOST ? ART_LOST_LOGO : ART_WIN_LOGO;

    char *scoreString = numToString(score >= 0 ? score : -score);
    char *scoreMessage = concat(2, partOfMessage, scoreString);

    int menuResult = generic_menu(screen, getArtOfThing(ART_UNKNOWN, choices, MENU_GAME_OVER_LEN),
                                  getArtOfThing(ART_UNKNOWN, &scoreMessage, 1),
                                  getArtOfThing(art, NULL, 0));
    free(scoreMessage);
    free(scoreString);

    return menuResult;
}

/**
 * Mostra un menu' in base allo stato del gioco.
 * @param screen    La struttura dello schermo.
 * @param state     Lo stato del gioco.
 * @param output    L'output del menu.
 * @return          0 se tutto è andato bene, 1 altrimenti.
 */
unsigned int show(const Screen screen, const enum PS state, int *output)
{
    clear_screen();

    switch (state)
    {
        case PS_MAIN_MENU: 
            *output = main_menu(screen);
            break;
        case PS_PAUSE_MENU: 
            *output = pause_menu(screen);
            break;
        case PS_VERSION_MENU:
            *output = thread_or_processes_menu(screen);
            break;
        case PS_WIN:
        case PS_LOST:
            *output = game_over_menu(screen, state, *output);
            break;
        default: 
            return 1;
    }

    clear_screen();
    return 0;
}
