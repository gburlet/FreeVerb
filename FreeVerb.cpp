/***************************************************/
/*! \class FreeVerb
    \brief Jezar at Dreampoint's FreeVerb implemented in STK

    TODO: class description here

    by Gregory Burlet, 2012.
*/
/***************************************************/

#include "FreeVerb.h"

using namespace stk;

FreeVerb::FreeVerb() {

    // resize lastFrame_ for stereo output
    lastFrame_.resize(1, 2, 0.0);

    numComb_ = 8;
    numAllPass_ = 4;

    // TODO: scale delayline lengths with sampling rate changes
    int cDelayLen[] = {1617, 1557, 1491, 1422, 1356, 1277, 1188, 1116};
    int aDelayLen[] = {556, 441, 341, 225};

    // pole of lowpass filters
    StkFloat d = 0.2;

    // feedback attenuation in LBFC
    feedback_ = 0.84;

    // allpass coefficient
    g_ = 0.5;

    // initialize delay lines for the LBFC filters
    for (int i = 0; i < numComb_; i++) {
        combDelayL_[i].setMaximumDelay(cDelayLen[i]);
        combDelayL_[i].setDelay(cDelayLen[i]);

        // set low pass filter for delay output
        combFilterL_[i].setCoefficients(1.0 - d, -d);
    }

    // initialize delay lines for the allpass filters
    for (int i = 0; i < numAllPass_; i++) {
        allPassDelayL_[i].setMaximumDelay(aDelayLen[i]);
        allPassDelayL_[i].setDelay(aDelayLen[i]);
    }
}

FreeVerb::~FreeVerb() {}

void FreeVerb::clear() {
    // clear LBFC delay lines
    for (int i = 0; i < numComb_; i++) {
        combDelayL_[i].clear();
    }

    // clear allpass delay lines
    for (int i = 0; i < numAllPass_; i++) {
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

StkFloat FreeVerb::tick(StkFloat input, unsigned int channel) {
    // 8 LBCF filters in parallel
    StkFloat allPassInputL = 0.0;
    for (int i = 0; i < numComb_; i++) {
        StkFloat yn = input + (feedback_ * combFilterL_[i].tick(combDelayL_[i].lastOut()));
        combDelayL_[i].tick(yn);
        allPassInputL += yn;
    }

    // 4 allpass filters in series
    for (int i = 0; i < numAllPass_; i++) {
        StkFloat vn_m = allPassDelayL_[i].lastOut();
        StkFloat vn = allPassInputL + (g_ * vn_m);
        allPassDelayL_[i].tick(vn);

        // calculate output
        allPassInputL = -vn + ((1.0 + g_) * vn_m);
    }

    allPassInputL *= 0.01;

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
