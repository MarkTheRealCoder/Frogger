cmake_minimum_required(VERSION 3.10.2)
project(frogger C)

set(CMAKE_C_STANDARD 11)

# utile per usare valgrind su linux
add_compile_options(-g -Og)

add_executable(frogger  src/main.c

                        src/commons/imports.h
                        src/commons/structures.h
                        src/commons/entities.h
                        src/commons/drawing.h

                        src/commons/drawings/drawing.c
                        src/commons/drawings/menus.c

                        src/utilities/addons.c
                        src/utilities/addons.h

                        src/utilities/shortcuts.c
                        src/utilities/shortcuts.h

                        src/utilities/common.c
                        src/utilities/common.h

                        src/concurrency/shared/shared.c
                        src/concurrency/shared/shared.h
                        
                        src/concurrency/processes.c
                        src/concurrency/processes.h
                        
                        src/concurrency/threads.c
                        src/concurrency/threads.h
)

# utile per aggiungere math.hpow() anche su gcc linux
target_link_libraries(frogger PRIVATE m pthread ncursesw)
