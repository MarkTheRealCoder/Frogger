#ifndef FROGGER_COMMON_H
#define FROGGER_COMMON_H

#include "../commons/imports.h"

#define COMPONENT_FROG 1

Entity entities_default_frog(MapSkeleton mapSkeleton);
Entity entities_default_plant(MapSkeleton mapSkeleton, int index);

struct entities_list *create_default_entities(GameSkeleton *game, int loadFromSkeleton);

#endif //FROGGER_COMMON_H
