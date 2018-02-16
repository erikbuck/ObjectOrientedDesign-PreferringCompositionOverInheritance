# This makefile builds the worms demonstration program by Erik Buck
# for CS7140-C01 Summer 2016 Wright State University

SOURCE_FILES=main.cpp \
    Worm.cpp \
    WormsSim.cpp \
    CursesWormsSimUIStrategy.cpp \
    Worm.h \
    WormsSim.h \
    CursesWormsSimUIStrategy.h

.PHONY: all
.PHONY: clean
.PHONY: docs

all: worms

worms: ${SOURCE_FILES} Makefile
	@echo "Building worms"
	c++ -std=c++14 -g *.cpp -o worms -lncurses -static-libstdc++

clean:
	@echo "Cleaning worms"
	rm -f *.o worms
	rm -rf *.dSYM

docs:   ../doxygen.config ${SOURCE_FILES} Makefile
	doxygen ../doxygen.config WSUWorms
