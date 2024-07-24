#include "drawing.h"


void init_screen(Screen *scrn) {
    initscr();
    noecho();
    curs_set(false);
    keypad(stdscr, true);
    start_color();
    getmaxyx(stdscr, scrn->y, scrn->x);
}

enum color_codes getEntityColor(const enum entity_type type) {
    enum color_codes color;
    switch (type) {
        case CROC_TYPE: color = COLORCODES_CROC_B;
            break;
        case CROC_ANGRY_TYPE: color = COLORCODES_CROC_A;
            break;
        case FLOWER_TYPE: color = COLORCODES_FLOWER_B;
            break;
        case FLOWER_HARMED_TYPE: color = COLORCODES_FLOWER_A;
            break;
        case FROG_TYPE: color = COLORCODES_FROG_B;
            break;
        case FROG_HARMED_TYPE: color = COLORCODES_FROG_A;
            break;
        case PROJ_FROG_TYPE: color = COLORCODES_PROJECTILE_F;
            break;
        case PROJ_FLOWER_TYPE: color = COLORCODES_PROJECTILE_FL;
            break;
    }
    return color;
}

enum areas getAreaFromPos(const Position pos) {
    
}

void eraseOldPosition(const enum color_codes areaColor, const Position old, const unsigned int length) {

}

EntityObject getEntityObject(const enum entity_type type) {
    return (EntityObject){.top = NULL, .middle = NULL, .bottom = NULL};
}

void drawNewEntity(const enum entity_type type, const enum color_codes areaColor, const Position new, const unsigned int length) {
    init_extended_pair(JOLLY_PAIR, getEntityColor(type), areaColor);
    EntityObject obj = getEntityObject(type);
    attron(COLOR_PAIR(JOLLY_PAIR));
    //todo Replace following
    mvaddstr(new.y, new.x, obj.top);
    mvaddstr(new.y+1, new.x, obj.middle);
    mvaddstr(new.y+2, new.x, obj.bottom);
    //
    attroff(COLOR_PAIR(JOLLY_PAIR));
}

void move_entity(const enum entity_type type, const Position old, const Position current, const unsigned int length) {
    enum areas oldArea = getAreaFromPos(old);
    enum areas newArea = getAreaFromPos(current);
    eraseOldPosition((enum color_codes)oldArea, old, length);
    drawNewEntity(type, (enum color_codes)newArea, current, length);
    refresh();
}





