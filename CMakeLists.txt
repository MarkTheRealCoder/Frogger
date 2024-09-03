cmake_minimum_required(VERSION 3.10.2)
project(frogger C)

set(CMAKE_C_STANDARD 11)

# utile per usare valgrind su linux
add_compile_options(-g -Og)

add_executable(frogger  src/main.c

        src/utils/imports.h

                        src/utils/shortcuts.c
                        src/utils/shortcuts.h

                        src/utils/addons.c
                        src/utils/addons.h
 
                         src/game/core.c
                         src/game/core.h
 
                         src/game/routines.c
                         src/game/routines.h
 
                         src/game/entities.c
                         src/game/entities.h
                         
                         src/graphics/drawings/drawing.c
                         src/graphics/drawings/menus.c
                         src/graphics/drawing.h
                         src/graphics/map.h
                         src/graphics/entities.h
                        
                         src/concurrency/processes.c
                         src/concurrency/processes.h

                         src/concurrency/threads.c
                         src/concurrency/threads.h

                         src/concurrency/common.c
                         src/concurrency/common.h
)

# utile per aggiungere math.hpow() anche su gcc linux
target_link_libraries(frogger PRIVATE m pthread ncursesw)
