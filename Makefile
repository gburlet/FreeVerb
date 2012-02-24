# MUMT 618 Final Project
# Stk FreeVerb implementation
# Gregory Burlet, 20120
#
# make file for the Stk FreeVerb algorithm

# links
LINKS = -I/Developer/stk-4.4.3/include/ -L/Developer/stk-4.4.3/src/

# libraries
LIBS = -lstk

# compiler flags
CFLAGS = -Wall

main: freeverb.o fvtest.o 
	g++ $(LINKS) freeverb.o fvtest.o -o fvTest $(LIBS)

freeverb.o: FreeVerb.h FreeVerb.cpp
	g++ -c $(CFLAGS) $(LINKS) FreeVerb.cpp -o freeverb.o

fvtest.o: FreeVerb.h FreeVerbTest.cpp
	g++ -c $(CFLAGS) $(LINKS) FreeVerbTest.cpp -o fvtest.o

clean:
	rm -rf *.o fvTest

