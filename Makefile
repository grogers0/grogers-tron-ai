# The Makefile
#
# If you're using Windows and you don't know what this file is,
# don't worry about it. Just use Visual C++ Express Edition or
# Dev-C++ to work on your code.

CXXFLAGS=-O2 -g
LINKFLAGS=

all: MyTronBot

OBJECTS = Map.o OpponentIsolated.o ReachableSquares.o GameTree.o

MyTronBot: ${OBJECTS} MyTronBot.o
	g++ ${CXXFLAGS} -o MyTronBot ${OBJECTS} MyTronBot.o ${LINKFLAGS}

%.o: %.cc
	g++ ${CXXFLAGS} -c $<

clean:
	rm -f *.o MyTronBot *.gcda core*
