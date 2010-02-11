# The Makefile
#
# If you're using Windows and you don't know what this file is,
# don't worry about it. Just use Visual C++ Express Edition or
# Dev-C++ to work on your code.

CXXFLAGS=-O2 -march=native

all: MyTronBot

OBJECTS = MyTronBot.o Map.o OpponentIsolated.o ReachableSquares.o

MyTronBot: ${OBJECTS}
	g++ ${CXXFLAGS} -o MyTronBot ${OBJECTS}

%.o: %.cc
	g++ ${CXXFLAGS} -c $<

clean:
	rm -f *.o MyTronBot
