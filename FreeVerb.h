#ifndef STK_FREEVERB_H
#define STK_FREEVERB_H

#include "Effect.h"
#include "Delay.h"
#include "OnePole.h"

namespace stk {

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

class FreeVerb : public Effect
{   
    public:
        //! Constructor
        FreeVerb();

        //! Destructor
        ~FreeVerb();

        void setMix(StkFloat value);

        void setRoomSize(StkFloat value);

        StkFloat getRoomSize();

        void setDamp(StkFloat value);

        StkFloat getDamp();

        void setWidth(StkFloat value);

        StkFloat getWidth();

        void setMode(bool isFrozen);

        StkFloat getMode();

        void update();

        void clear();

        StkFloat lastOut(unsigned int channel = 0);

        StkFloat tick(StkFloat inputL, StkFloat inputR = 0.0, unsigned int channel = 0);

        StkFrames& tick(StkFrames& frames);

        StkFrames& tick(StkFrames& iFrames, StkFrames &oFrames);
    
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
        StkFloat roomSizeMem_, roomSize_;
        StkFloat dampMem_, damp_;
        StkFloat wet1_, wet2_;
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
