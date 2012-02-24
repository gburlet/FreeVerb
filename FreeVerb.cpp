/***************************************************/
/*! \class FreeVerb
    \brief Jezar at Dreampoint's FreeVerb implemented in STK

    TODO: class description here

    by Gregory Burlet, 2012.
*/
/***************************************************/

#include "FreeVerb.h"
#include <math.h>

using namespace stk;

FreeVerb::FreeVerb() {
    // resize lastFrame_ for stereo output
    lastFrame_.resize(1, 2, 0.0);

    // initialize parameters
    setWet(1.0 / scaleWet);
    setRoomSize(0.5);       // feedback attenuation in LBFC
    setDry(0.0);
    setDamp(0.5);           // pole of lowpass filters in he LBFC
    setWidth(1.0);
    setMode(false);

    gain_ = fixedGain;      // input gain before sending to filters
    g_ = 0.5;               // allpass coefficient, immutable in FreeVerb

    // delay line lengths for 44100Hz sampling rate
    int cDelayLen[numCombs] = {1617, 1557, 1491, 1422, 1356, 1277, 1188, 1116};
    int aDelayLen[numAllPasses] = {556, 441, 341, 225};

    // scale delay line lengths according to the current sampling rate
    double scaler = Stk::sampleRate() / 44100.0;
    if (scaler != 1.0) {
        // scale comb filter delay lines
        for (int i = 0; i < numCombs; i++) {
            cDelayLen[i] = (int) floor(scaler * cDelayLen[i]);
        }

        // scale allpass filter delay lines
        for (int i = 0; i < numAllPasses; i++) {
            aDelayLen[i] = (int) floor(scaler * aDelayLen[i]);
        }
    }

    // initialize delay lines for the LBFC filters
    for (int i = 0; i < numCombs; i++) {
        combDelayL_[i].setMaximumDelay(cDelayLen[i]);
        combDelayL_[i].setDelay(cDelayLen[i]);

        // set low pass filter for delay output
        combFilterL_[i].setCoefficients(1.0 - damp_, -damp_);
    }

    // initialize delay lines for the allpass filters
    for (int i = 0; i < numAllPasses; i++) {
        allPassDelayL_[i].setMaximumDelay(aDelayLen[i]);
        allPassDelayL_[i].setDelay(aDelayLen[i]);
    }
}

FreeVerb::~FreeVerb() {}

void FreeVerb::setRoomSize(StkFloat roomSize) {
    if (!frozenMode_) {
        roomSize_ = (roomSize * scaleRoom) + offsetRoom;
    }
}

StkFloat FreeVerb::getRoomSize() {
    return (roomSize_ - offsetRoom) / scaleRoom;
}

void FreeVerb::setDamp(StkFloat damping) {
    if (!frozenMode_) {
        damp_ = damping * scaleDamp;
    }
}

StkFloat FreeVerb::getDamp() {
    return damp_ / scaleDamp;
}

void FreeVerb::setWet(StkFloat wet) {
    wet_ = wet * scaleWet;

    wet1_ = wet_ * ((width_ / 2.0) + 0.5);
	wet2_ = wet_ * ((1.0 - width_) / 2.0);
}

StkFloat FreeVerb::getWet() {
    return wet_ / scaleWet;
}

void FreeVerb::setDry(StkFloat dry) {
    dry_ = dry * scaleDry;
}

StkFloat FreeVerb::getDry() {
    return dry_ / scaleDry;
}

void FreeVerb::setWidth(StkFloat width) {
    width_ = width;

    // width affects wet1 and wet2 parameters
    wet1_ = wet_ * ((width_ / 2.0) + 0.5);
	wet2_ = wet_ * ((1.0 - width_) / 2.0);
}

StkFloat FreeVerb::getWidth() {
    return width_;
}

void FreeVerb::setMode(bool isFrozen) {
    frozenMode_ = isFrozen;

    // declare frozen memory
    static StkFloat roomSizeMem = roomSize_;
    static StkFloat dampMem = damp_;

    if (frozenMode_) {
        // save state
        roomSizeMem = roomSize_;
        dampMem = damp_;

        // put into freeze mode
        roomSize_ = 1.0;
        damp_ = 0.0;
        gain_ = 0.0;
    }
    else {
        roomSize_ = roomSizeMem;
        damp_ = dampMem;
        gain_ = fixedGain;
    }
}

StkFloat FreeVerb::getMode() {
    return frozenMode_;
}

void FreeVerb::clear() {
    // clear LBFC delay lines
    for (int i = 0; i < numCombs; i++) {
        combDelayL_[i].clear();
    }

    // clear allpass delay lines
    for (int i = 0; i < numAllPasses; i++) {
        allPassDelayL_[i].clear();
    }

    lastFrame_[0] = 0.0;
    lastFrame_[1] = 0.0;
}

StkFloat FreeVerb::lastOut(unsigned int channel) {
#if defined(_STK_DEBUG_)
    if (channel > 1) {
        oStream_ << "FreeVerb::lastOut(): channel argument must be less than 2!";
        handleError(StkError::FUNCTION_ARGUMENT);
    }
#endif

    return lastFrame_[channel];
}

// TODO: make it take two input channels
StkFloat FreeVerb::tick(StkFloat input, unsigned int channel) {
    // gain
    input *= gain_;

    // 8 LBCF filters in parallel
    StkFloat allPassInputL = 0.0;
    for (int i = 0; i < numCombs; i++) {
        StkFloat yn = input + (roomSize_ * combFilterL_[i].tick(combDelayL_[i].lastOut()));
        combDelayL_[i].tick(yn);
        allPassInputL += yn;
    }

    // 4 allpass filters in series
    for (int i = 0; i < numAllPasses; i++) {
        StkFloat vn_m = allPassDelayL_[i].lastOut();
        StkFloat vn = allPassInputL + (g_ * vn_m);
        allPassDelayL_[i].tick(vn);

        // calculate output
        allPassInputL = -vn + ((1.0 + g_) * vn_m);
    }

    lastFrame_[0] = allPassInputL;
    lastFrame_[1] = allPassInputL;

    return lastFrame_[channel];
}

StkFrames& FreeVerb::tick(StkFrames& frames, unsigned int channel) {
    return frames;
}

StkFrames& FreeVerb::tick(StkFrames& iFrames, StkFrames &oFrames, unsigned int iChannel, unsigned int oChannel) {
    return oFrames;
}
