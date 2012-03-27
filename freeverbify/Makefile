# MUMT 618 Final Project
# Stk FreeVerb implementation
# Gregory Burlet, 20120
#
# make file for a test program which applies FreeVerb to an audio file

FREEVERB_PATH = ..
OBJECT_PATH = Release
OBJECTS	= freeverb.o freeverbify.o
vpath %.o $(OBJECT_PATH)

# links
LINKS = -I/Developer/stk-4.4.3/include/ -L/Developer/stk-4.4.3/src/

# libraries
LIBS = -lstk

# compiler flags
CFLAGS = -Wall

freeverbify: $(OBJECTS)
	g++ $(LINKS) $(OBJECT_PATH)/*.o -o $@ $(LIBS)

freeverb.o: $(FREEVERB_PATH)/FreeVerb.cpp $(FREEVERB_PATH)/FreeVerb.h
	g++ -c $(CFLAGS) $(LINKS) $< -o $(OBJECT_PATH)/$@

freeverbify.o: FreeVerbify.cpp $(FREEVERB_PATH)/FreeVerb.h
	g++ -c $(CFLAGS) $(LINKS) $< -o $(OBJECT_PATH)/$@

$(OBJECTS): | $(OBJECT_PATH)

$(OBJECT_PATH):
	mkdir $(OBJECT_PATH)

clean:
	rm -rf $(OBJECT_PATH) freeverbify

