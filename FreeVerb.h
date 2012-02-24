#ifndef STK_FREEVERB_H
#define STK_FREEVERB_H

#include "Effect.h"
#include "Delay.h"
#include "OnePole.h"

namespace stk {

/***************************************************/
/*! \class FreeVerb
    \brief Jezar at Dreampoint's FreeVerb implemented in STK

    TODO: class description here

    by Gregory Burlet, 2012.
*/
/***************************************************/

class FreeVerb : public Effect
{   
    public:
        //! Constructor
        FreeVerb();

        //! Destructor
        ~FreeVerb();

        void setRoomSize(StkFloat value);

        StkFloat getRoomSize();

        void setDamp(StkFloat value);

        StkFloat getDamp();

        void setWet(StkFloat value);

        StkFloat getWet();

        void setDry(StkFloat value);

        StkFloat getDry();

        void setWidth(StkFloat value);

        StkFloat getWidth();

        void setMode(bool isFrozen);

        StkFloat getMode();

        void clear();

        StkFloat lastOut(unsigned int channel = 0);

        StkFloat tick(StkFloat input, unsigned int channel = 0);

        StkFrames& tick(StkFrames& frames, unsigned int channel = 0);

        StkFrames& tick(StkFrames& iFrames, StkFrames &oFrames, unsigned int iChannel = 0, unsigned int oChannel =0);
    
        static const int numCombs = 8;
        static const int numAllPasses = 4;
        static const int stereoSpread = 23;
        static const StkFloat fixedGain = 0.015;
        static const StkFloat scaleWet = 3;
        static const StkFloat scaleDry = 2;
        static const StkFloat scaleDamp = 0.4;
        static const StkFloat scaleRoom = 0.28;
        static const StkFloat offsetRoom = 0.7;

        // delay line lengths for 44100Hz sampling rate
        static int cDelayLen[numCombs];
        static int aDelayLen[numAllPasses];

    protected:
        StkFloat g_;        // allpass coefficient
        StkFloat gain_;
        StkFloat roomSize_;
        StkFloat damp_;
        StkFloat wet_, wet1_, wet2_;
        StkFloat dry_;
        StkFloat width_;
        bool frozenMode_;

        // LBFC: Lowpass Feedback Comb Filters
        Delay combDelayL_[numCombs];
        Delay combDelayR_[numCombs];
        OnePole combFilterL_[numCombs];
        OnePole combFilterR_[numCombs];
        
        // AP: Allpass Filters
        Delay allPassDelayL_[numAllPasses];
        Delay allPassDelayR_[numAllPasses];
};

}

#endif
