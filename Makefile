# The Makefile
#
# If you're using Windows and you don't know what this file is,
# don't worry about it. Just use Visual C++ Express Edition or
# Dev-C++ to work on your code.

CXXFLAGS=-O2 -march=native -pg -Wall -g
LINKFLAGS=

all: MyTronBot

OBJECTS = Map.o OpponentIsolated.o ReachableSquares.o GameTree.o Time.o

MyTronBot: ${OBJECTS} MyTronBot.o
	g++ ${CXXFLAGS} -o MyTronBot ${OBJECTS} MyTronBot.o ${LINKFLAGS}

%.o: %.cc
	g++ ${CXXFLAGS} -c $<

clean:
	rm -f *.o MyTronBot Replay
