CC = gcc
CFLAGS = -g -O0 -Wall -Wno-unused

LIBS = -lncursesw -lpthread
INCLUDES = -Isrc/utilities -Isrc/commons -Isrc/concurrency -Isrc/concurrency/shared

OBJS = 	out/main.o 		\
		out/drawing.o 	\
		out/menus.o 	\
		out/threads.o 	\
		out/processes.o \
		out/shared.o 	\
		out/shortcuts.o \
		out/common.o 	\
		out/addons.o

all: outdir frogger
	@echo "Compilation done!"

outdir:
	mkdir -p out

frogger: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o frogger $(LIBS)

# MAIN

out/main.o: src/main.c
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -c src/main.c -o out/main.o

# COMMONS

out/drawing.o: src/commons/drawings/drawing.c
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -c src/commons/drawings/drawing.c -o out/drawing.o

out/menus.o: src/commons/drawings/menus.c
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -c src/commons/drawings/menus.c -o out/menus.o

# CONCURRENCY

out/threads.o: src/concurrency/threads.c
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -c src/concurrency/threads.c -o out/threads.o

out/processes.o: src/concurrency/processes.c
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -c src/concurrency/processes.c -o out/processes.o

out/shared.o: src/concurrency/shared/shared.c
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -c src/concurrency/shared/shared.c -o out/shared.o

# UTILITIES

out/shortcuts.o: src/utilities/shortcuts.c
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -c src/utilities/shortcuts.c -o out/shortcuts.o

out/common.o: src/utilities/common.c
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -c src/utilities/common.c -o out/common.o

out/addons.o: src/utilities/addons.c
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -c src/utilities/addons.c -o out/addons.o


clean:
	rm -r out/ frogger