#ifndef FROGGER_COMMON_H
#define FROGGER_COMMON_H

#include "../commons/imports.h"

/*
 * Indexes dei componenti.
 */

#define COMPONENT_FROG_INDEX                        0
#define COMPONENT_CROC_INDEXES                      16 // da 1 a 16
#define COMPONENT_CLOCK_INDEX                       20
#define COMPONENT_TEMPORARY_CLOCK_INDEX             COMPONENT_CLOCK_INDEX + 1
#define COMPONENT_FROG_PROJECTILES_INDEX            COMPONENT_TEMPORARY_CLOCK_INDEX + 1
#define COMPONENT_PROJECTILES_INDEX                 COMPONENT_FROG_PROJECTILES_INDEX + 1

Entity entities_default_frog(MapSkeleton map);
Entity entities_default_plant();

struct entities_list *create_default_entities(GameSkeleton *game);
Clock *create_clock(unsigned int value, enum ClockType type);
Entities *create_entities_group();
void create_new_entities(struct entities_list **list, Component components[MAX_CONCURRENCY], MapSkeleton map);

#endif //FROGGER_COMMON_H
