# MUMT 618 Final Project
# Stk FreeVerb implementation
# Gregory Burlet, 2012
#
# make file for the program with a front end for setting FreeVerb
# parameters with realtime audio input.

SRC_PATH = /Developer/stk-4.4.3/src
FREEVERB_PATH = ..
OBJECT_PATH = Release
vpath %.o $(OBJECT_PATH)

OBJECTS	= freeverb.o freeverbgui.o
 
# links
LINKS = -I/Developer/stk-4.4.3/include/ -L/Developer/stk-4.4.3/src/

# statically compiled libraries
SLIBS = -lstk

DEFS = -DHAVE_GETTIMEOFDAY -D__MACOSX_CORE__ -D__LITTLE_ENDIAN__
CFLAGS = -O3 -Wall
LIBRARY = -lpthread -framework CoreAudio -framework CoreFoundation -framework CoreMidi

freeverbgui: $(OBJECTS)
	g++ $(CFLAGS) $(LINKS) $(DEFS) $(OBJECT_PATH)/*.o -o $@ $(SLIBS) $(LIBRARY)

freeverb.o: $(FREEVERB_PATH)/FreeVerb.cpp $(FREEVERB_PATH)/FreeVerb.h
	g++ -c $(CFLAGS) $(LINKS) $< -o $(OBJECT_PATH)/$@

freeverbgui.o: FreeVerbGUI.cpp $(FREEVERB_PATH)/FreeVerb.h
	g++ -c $(CFLAGS) $(LINKS) $(DEFS) $< -o $(OBJECT_PATH)/$@

$(OBJECTS): | $(OBJECT_PATH)

$(OBJECT_PATH):
	mkdir $(OBJECT_PATH)

clean: 
	rm -rf $(OBJECT_PATH) freeverbgui

run:
	wish < tcl/FreeVerbGUI.tcl | ./freeverbgui -ip
