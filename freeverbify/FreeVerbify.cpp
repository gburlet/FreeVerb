/*
 * MUMT 618 Final project
 * Gregory Burlet, 2012
 *
 * This is a functional test program for the Stk implementation of FreeVerb.
 */

#include <iostream>
#include <string>

#include "FileWvIn.h"
#include "FileWvOut.h"
#include "../FreeVerb.h"

using namespace stk;

int main(int argc, char *argv[]) {
    FileWvIn input;
    FileWvOut output;

    if (argc != 3) {
        std::cout << "usage: " << argv[0] << " filein fileout" << std::endl;
        std::cout << "  where 'filein' is an input soundfile to process and 'fileout' is where to write the output soundfile" << std::endl;
        exit(0);
    }

    // Load the input sound file
    try {
        input.openFile(argv[1]);
    }
    catch (StkError &) {
        exit(0);
    }

    // Set global sample rate before creating class instances
    Stk::setSampleRate(input.getFileRate());
    input.setRate(1.0);

    // Open an output file for writing
    try {
        output.openFile(argv[2], input.channelsOut(), FileWrite::FILE_AIF, Stk::STK_SINT16);
    }
    catch (StkError &) {
        input.closeFile();
        exit(0);
    }

    FreeVerb fv = FreeVerb();
    fv.setDamp(0.20);
    fv.setWidth(0.5);
    fv.setRoomSize(0.75);
    fv.setMix(0.75);

    /* single sample computation */
    /*
    for (unsigned int i = 0; i < input.getSize(); i++) {
        try {
            output.tick(fv.tick(input.tick()));
        }
        catch (StkError &) {
            break; // file pointer cleanup
        }
    }*/
    
    /* use STK frames with replacement
    StkFrames frames(input.getSize(), input.channelsOut());
    try {
        output.tick(fv.tick(input.tick(frames)));
    }
    catch (StkError &) {
        // file pointer cleanup
    }*/

    // using STK frames with seperate in and outs
    StkFrames iFrames(input.getSize(), input.channelsOut());
    StkFrames oFrames(input.getSize(), input.channelsOut());
    try {
        output.tick(fv.tick(input.tick(iFrames), oFrames));
    }
    catch (StkError &) {
        // file pointer cleanup
    }

    input.closeFile();
    output.closeFile();
}
