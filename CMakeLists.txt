cmake_minimum_required(VERSION 3.10.2)
project(frogger C)

set(CMAKE_C_STANDARD 11)

# utile per usare valgrind su linux
add_compile_options(-g -Og)

add_executable(frogger  src/main.c

                        src/utils/imports.h
                        src/utils/shortcuts.c
                        src/utils/shortcuts.h
                        
                        src/game/addons.c
                        src/game/addons.h

                        src/game/core.c
                        src/game/core.h

                        src/game/routines.c
                        src/game/routines.h
)

# utile per aggiungere math.h#pow() anche su gcc linux
target_link_libraries(frogger PRIVATE m pthread ncursesw)
