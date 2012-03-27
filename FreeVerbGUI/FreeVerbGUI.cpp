/**************  Effects Program  *********************/

#include "Skini.h"
#include "SKINI.msg"
#include "Envelope.h"
#include "../FreeVerb.h"
#include "Messager.h"
#include "RtAudio.h"

#include <signal.h>
#include <cstring>
#include <iostream>
#include <algorithm>

using namespace stk;

// default sample frames between control input checks
#define DELTA_CONTROL_TICKS 64

void usage(void) {
    // Error function in case of incorrect command-line argument specifications
    std::cout << std::endl << "usage: effects flags" << std::endl;
    std::cout << "\twhere flag = -s RATE to specify a sample rate," << std::endl;
    std::cout << "\tflag = -ip for realtime SKINI input by pipe" << std::endl;
    std::cout << "\t\t(won't work under Win95/98)," << std::endl;
    std::cout << "\tand flag = -is <port> for realtime SKINI input by socket." << std::endl;
    exit(0);
}

bool done;
/*
 * Interrupt handler
 */
static void finish(int ignore) { 
    done = true;
}

/*
 The TickData structure holds all the class instances and data that
 are shared by the various processing functions.
*/
class TickData {
    public:
        TickData()
        : counter(0), haveMessage(false) {}

        FreeVerb freerev;
        Envelope envelope;
        Messager messager;
        Skini::Message message;
        StkFloat lastSample;
  
        int counter;
        bool haveMessage;
};

/*
 * The processMessage() function encapsulates the handling of control
 * messages.  It can be easily relocated within a program structure
 * depending on the desired scheduling scheme.
 */
void processMessage(TickData* data) {
    register unsigned int msgID = data->message.intValues[0];
    register StkFloat valueMIDI = data->message.floatValues[1];
    register StkFloat value = valueMIDI * ONE_OVER_128;

    switch(data->message.type) {
        case __SK_Exit_:
            data->envelope.setTarget(0.0);
            done = true;
            return;
        case __SK_NoteOn_:
            if (valueMIDI == 0.0) {
                // really a NoteOff
                data->envelope.setTarget(0.0);
            }
            else {
                // a NoteOn
                data->envelope.setTarget(1.0);
            }
            break;
        case __SK_NoteOff_:
            data->envelope.setTarget(0.0);
            break;
        case __SK_ControlChange_:
            switch (msgID) {
                case 22: 
                    // parameter room size change
                    data->freerev.setRoomSize(value);
                    break;
                case 23:
                    // parameter damping change
                    data->freerev.setDamp(value);
                    break;
                case 24:
                    // parameter width change
                    data->freerev.setWidth(value);
                    break;
                case 25:
                    // parameter freeze mode change
                    if (value > 0.5) {
                        data->freerev.setMode(true);
                    }
                    else {
                        data->freerev.setMode(false);
                    }
                    break;
                case 44:
                    // parameter effect mix change
                    data->freerev.setMix(value);
                    break; 
            }
    }

    data->haveMessage = false;
    return;
}

/*
 * The tick() function handles sample computation and scheduling of
 * control updates.  It will be called automatically by RtAudio when
 * the system needs a new buffer of audio samples.
 */
int tick(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *dataPointer) {
    TickData *data = (TickData *) dataPointer;
    register StkFloat *oSamples = (StkFloat *) outputBuffer, *iSamples = (StkFloat *) inputBuffer;
    Effect *effect;

    int counter, nTicks = (int) nBufferFrames;
    while (nTicks > 0 && !done) {
        if (!data->haveMessage) {
            data->messager.popMessage(data->message);
            if (data->message.type > 0) {
                data->counter = (long) (data->message.time * Stk::sampleRate());
                data->haveMessage = true;
            }
            else {
                data->counter = DELTA_CONTROL_TICKS;
            }
        }

        counter = std::min(nTicks, data->counter);
        data->counter -= counter;
        for (int i = 0; i < counter; i++) {
            data->freerev.tick(*iSamples++);
            effect = (Effect *) &(data->freerev);
            const StkFrames& samples = effect->lastFrame();
            *oSamples++ = data->envelope.tick() * samples[0];
            *oSamples++ = data->envelope.lastOut() * samples[1];
            nTicks--;
        }

        if (nTicks == 0) {
            break;
        }

        // Process control messages.
        if (data->haveMessage) {
            processMessage(data);
        }
  }

  return 0;
}

int main(int argc, char *argv[]) {
    TickData data;
    RtAudio adac;

    if (argc < 2 || argc > 6) {
        usage();
    }

    // If you want to change the default sample rate (set in Stk.h), do
    // it before instantiating any objects!  If the sample rate is
    // specified in the command line, it will override this setting.
    Stk::setSampleRate(44100.0);

    // Parse the command-line arguments.
    unsigned int port = 2001;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-is")) {
            if (i+1 < argc && argv[i+1][0] != '-') {
                port = atoi(argv[++i]);
            }
            data.messager.startSocketInput(port);
        }
        else if (!strcmp(argv[i], "-ip")) {
            data.messager.startStdInput();
        }
        else if (!strcmp(argv[i], "-s") && (i+1 < argc) && argv[i+1][0] != '-') {
            Stk::setSampleRate(atoi(argv[++i]));
        }
        else {
            usage();
        }
    }

    // Allocate the adac here.
    RtAudioFormat format = (sizeof(StkFloat) == 8) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
    RtAudio::StreamParameters oparameters, iparameters;
    oparameters.deviceId = adac.getDefaultOutputDevice();
    oparameters.nChannels = 2;
    iparameters.deviceId = adac.getDefaultInputDevice();
    iparameters.nChannels = 1;
    unsigned int bufferFrames = RT_BUFFER_SIZE;
    try {
        adac.openStream(&oparameters, &iparameters, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &tick, (void *)&data);
    }
    catch (RtError& error) {
        error.printMessage();
        goto cleanup;
    }

    data.envelope.setRate(0.001);

    // Install an interrupt handler function.
	(void) signal( SIGINT, finish );

    // If realtime output, set our callback function and start the dac.
    try {
        adac.startStream();
    }
    catch (RtError &error) {
        error.printMessage();
        goto cleanup;
    }

    // Setup finished.
    // Periodically check "done" status.
    while (!done) {
        Stk::sleep( 50 );
    }

    // Shut down the output stream.
    try {
        adac.closeStream();
    }
    catch (RtError& error) {
        error.printMessage();
    }

    cleanup:
	    std::cout << std::endl << "effects finished ... goodbye." << std::endl;

    return 0;
}

