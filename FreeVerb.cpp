/**************************************************************************/
/*! \class FreeVerb
    \brief Jezar at Dreampoint's FreeVerb, implemented in STK

    Freeverb is a free and open-source Schroeder 
    reverberator originally implemented in C++, and now
    implemented in Stk. The parameters of the reverberation model are
    exceptionally well tuned. FreeVerb uses 8 lowpass-feedback-comb-filters
    in parallel, followed by 4 Schroeder allpass filters in series.
    The input signal can be either mono or stereo, and the output signal
    is stereo.

    ported by Gregory Burlet, 2012.
*/
/***************************************************************************/

#include "FreeVerb.h"
#include <math.h>

using namespace stk;

// set static delay line lengths
int FreeVerb::cDelayLen[] = {1617, 1557, 1491, 1422, 1356, 1277, 1188, 1116};
int FreeVerb::aDelayLen[] = {225, 556, 441, 341};

FreeVerb::FreeVerb() {
    // resize lastFrame_ for stereo output
    lastFrame_.resize(1, 2, 0.0);

    // initialize parameters
    this->setMix(0.75);            // set initially to 3/4 wet 1/4 dry signal (different than original freeverb) 
    this->setRoomSize(0.75);       // feedback attenuation in LBFC
    this->setDamp(0.25);           // pole of lowpass filters in the LBFC
    this->setWidth(1.0);
    this->setMode(false);

    gain_ = fixedGain;      // input gain before sending to filters
    g_ = 0.5;               // allpass coefficient, immutable in FreeVerb

    // scale delay line lengths according to the current sampling rate
    double fsScale = Stk::sampleRate() / 44100.0;
    if (fsScale != 1.0) {
        // scale comb filter delay lines
        for (int i = 0; i < numCombs; i++) {
            cDelayLen[i] = (int) floor(fsScale * cDelayLen[i]);
        }

        // scale allpass filter delay lines
        for (int i = 0; i < numAllPasses; i++) {
            aDelayLen[i] = (int) floor(fsScale * aDelayLen[i]);
        }
    }

    // initialize delay lines for the LBFC filters
    for (int i = 0; i < numCombs; i++) {
        combDelayL_[i].setMaximumDelay(cDelayLen[i]);
        combDelayL_[i].setDelay(cDelayLen[i]);
        combDelayR_[i].setMaximumDelay(cDelayLen[i] + stereoSpread);
        combDelayR_[i].setDelay(cDelayLen[i] + stereoSpread);
    }

    // initialize delay lines for the allpass filters
    for (int i = 0; i < numAllPasses; i++) {
        allPassDelayL_[i].setMaximumDelay(aDelayLen[i]);
        allPassDelayL_[i].setDelay(aDelayLen[i]);
        allPassDelayR_[i].setMaximumDelay(aDelayLen[i] + stereoSpread);
        allPassDelayR_[i].setDelay(aDelayLen[i] + stereoSpread);
    }
}

FreeVerb::~FreeVerb() {}

void FreeVerb::setMix(StkFloat value) {
    this->setEffectMix(value);
    update();    
}

void FreeVerb::setRoomSize(StkFloat roomSize) {
    roomSizeMem_ = (roomSize * scaleRoom) + offsetRoom;
    update();
}

StkFloat FreeVerb::getRoomSize() {
    return (roomSizeMem_ - offsetRoom) / scaleRoom;
}

void FreeVerb::setDamp(StkFloat damping) {
    dampMem_ = damping * scaleDamp;
    update();
}

StkFloat FreeVerb::getDamp() {
    return dampMem_ / scaleDamp;
}

void FreeVerb::setWidth(StkFloat width) {
    width_ = width;
    update();
}

StkFloat FreeVerb::getWidth() {
    return width_;
}

void FreeVerb::setMode(bool isFrozen) {
    frozenMode_ = isFrozen;
    update();
}

StkFloat FreeVerb::getMode() {
    return frozenMode_;
}

void FreeVerb::update() {
    StkFloat wet = scaleWet * effectMix_;
    wet1_ = wet * (width_/2.0 + 0.5);
    wet2_ = wet * (1.0 - width_)/2;

    if (frozenMode_) {
        // put into freeze mode
        roomSize_ = 1.0;
        damp_ = 0.0;
        gain_ = 0.0;
    }
    else {
        roomSize_ = roomSizeMem_;
        damp_ = dampMem_;
        gain_ = fixedGain;
    }

    for (int i = 0; i < numCombs; i++) {
        // set low pass filter for delay output
        combFilterL_[i].setCoefficients(1.0 - damp_, -damp_);
        combFilterR_[i].setCoefficients(1.0 - damp_, -damp_);
    }
}

void FreeVerb::clear() {
    // clear LBFC delay lines
    for (int i = 0; i < numCombs; i++) {
        combDelayL_[i].clear();
        combDelayR_[i].clear();
    }

    // clear allpass delay lines
    for (int i = 0; i < numAllPasses; i++) {
        allPassDelayL_[i].clear();
        allPassDelayR_[i].clear();
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

StkFloat FreeVerb::tick(StkFloat inputL, StkFloat inputR, unsigned int channel) {
#if defined(_STK_DEBUG_)
    if (channel > 1) {
        oStream_ << "FreeVerb::tick(): channel argument must be less than 2!";
        handleError(StkError::FUNCTION_ARGUMENT);
    }
#endif

    if (!inputR) {
        inputR = inputL;
    }

    // gain
    StkFloat fInput = (inputL + inputR) * gain_;

    StkFloat outL = 0.0;
    StkFloat outR = 0.0;

    // 8 LBCF filters in parallel
    for (int i = 0; i < numCombs; i++) {
        // process L channel
        StkFloat yn = fInput + (roomSize_ * combFilterL_[i].tick(combDelayL_[i].nextOut()));
        combDelayL_[i].tick(yn);
        outL += yn;

        // process R channel
        yn = fInput + (roomSize_ * combFilterR_[i].tick(combDelayR_[i].nextOut()));
        combDelayR_[i].tick(yn);
        outR += yn;
    }

    // 4 allpass filters in series
    for (int i = 0; i < numAllPasses; i++) {
        // process L channel
        StkFloat vn_m = allPassDelayL_[i].nextOut();
        StkFloat vn = outL + (g_ * vn_m);
        allPassDelayL_[i].tick(vn);
        
        // calculate output
        outL = -vn + (1.0 + g_)*vn_m;

        // process R channel
        vn_m = allPassDelayR_[i].nextOut();
        vn = outR + (g_ * vn_m);
        allPassDelayR_[i].tick(vn);

        // calculate output
        outR = -vn + (1.0 + g_)*vn_m;
    }

    // mix output
    lastFrame_[0] = outL*wet1_ + outR*wet2_ + inputL*scaleDry*(1.0-effectMix_);
    lastFrame_[1] = outR*wet1_ + outL*wet2_ + inputR*scaleDry*(1.0-effectMix_);

    return lastFrame_[channel];
}

StkFrames& FreeVerb::tick(StkFrames& frames) {
    unsigned int numChannels = frames.channels();

#if defined(_STK_DEBUG_)
    if (numChannels > 2) {
        oStream_ << "FreeVerb::tick(): must be <= 2 channels!";
        handleError(StkError::FUNCTION_ARGUMENT);
    }
#endif

    StkFloat *samples = &frames[0];
    for (unsigned int i = 0; i < frames.frames(); i++, samples += numChannels) {
        // if frames is stereo
        if (numChannels == 2) {
            *samples = tick(*samples, *(samples+1));
            *(samples+1) = lastFrame_[1];
        }
        else {
            *samples = tick(*samples);
        }
    }

    return frames;
}

StkFrames& FreeVerb::tick(StkFrames& iFrames, StkFrames &oFrames) {
    unsigned int iNumChannels = iFrames.channels();
    unsigned int oNumChannels = oFrames.channels();

#if defined(_STK_DEBUG_)
    if (iNumChannels > 2 || oNumChannels > 2) {
        oStream_ << "FreeVerb::tick(): must be <= 2 channels!";
        handleError(StkError::FUNCTION_ARGUMENT);
    }
#endif

    StkFloat *iSamples = &iFrames[0];
    StkFloat *oSamples = &oFrames[0];
    for (unsigned int i = 0; i < iFrames.frames(); i++, iSamples += iNumChannels, oSamples += oNumChannels) {
        // if iFrames is stereo
        if (iNumChannels == 2) {
            *oSamples = tick(*iSamples, *(iSamples+1));
        }
        else {
            *oSamples = tick(*iSamples);
        }

        // if oFrames is stereo
        if(oNumChannels == 2) {
            *(oSamples+1) = lastFrame_[1];
        }
    }

    return oFrames;
}
