# MUMT 618 Final Project
# Stk FreeVerb implementation
# Gregory Burlet, 20120
#
# make file for a test program which applies FreeVerb to an audio file

FREEVERB_PATH = ..
OBJECT_PATH = Release

# links
LINKS = -I/Developer/stk-4.4.3/include/ -L/Developer/stk-4.4.3/src/

# libraries
LIBS = -lstk

# compiler flags
CFLAGS = -Wall

main: freeverb.o fvtest.o 
	g++ $(LINKS) $(OBJECT_PATH)/freeverb.o $(OBJECT_PATH)/fvtest.o -o fvAudio $(LIBS)

freeverb.o: $(FREEVERB_PATH)/FreeVerb.h $(FREEVERB_PATH)/FreeVerb.cpp
	g++ -c $(CFLAGS) $(LINKS) $(FREEVERB_PATH)/FreeVerb.cpp -o $(OBJECT_PATH)/freeverb.o

fvtest.o: $(FREEVERB_PATH)/FreeVerb.h FreeVerbTest.cpp
	g++ -c $(CFLAGS) $(LINKS) FreeVerbTest.cpp -o $(OBJECT_PATH)/fvtest.o

clean:
	rm -rf $(OBJECT_PATH)/*.o fvAudio

