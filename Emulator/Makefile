CC=g++
SOURCES=emulator.cpp 
FILES := $(subst .cpp,,$(SOURCES))
FLAGS=-g -Wall -Wvla -Werror -Wno-unused-variable -Wno-unused-but-set-variable -DEMULATE
LIBS=-lcurses

all: $(FILES)

% :: %.cpp
	$(CC) $(FLAGS) $< -o $@ -lcurses

clean:
	rm -rf $(FILES)

