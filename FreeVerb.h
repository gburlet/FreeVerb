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
        //! FreeVerb Constructor
        /*!
          Initializes the effect with default parameters. Note that these defaults
          are slightly different than the defaults provided in the original 
          implementation in FreeVerb.
          Defaults:
            Effect Mix: 0.75
            Room Size: 0.75
            Damping: 0.25
            Width: 1.0
            Mode: freeze mode off
        */
        FreeVerb();

        //! Destructor
        ~FreeVerb();

        //! set the effect mix [0,1]
        /*!
         value = 1.0 corresponds to 100% wet mix, 0.0 corresponds to 100% dry mix
        */
        void setMix(StkFloat value);

        //! set the room size parameter [0,1]
        void setRoomSize(StkFloat value);

        //! get the room size parameter
        StkFloat getRoomSize();

        //! set the damping parameter [0,1]
        void setDamp(StkFloat value);

        //! get the damping parameter
        StkFloat getDamp();

        //! set the width parameter [0,1]
        void setWidth(StkFloat value);

        //! get the width parameter
        StkFloat getWidth();

        //! set the mode, frozen or not
        void setMode(bool isFrozen);

        //! get the current freeze mode
        StkFloat getMode();

        //! update parameters
        /*!
          Since some changes in parameters are interdependent,
          this keeps everything in sync.
        */
        void update();

        //! clears delay lines, etc.
        void clear();

        //! returns the last calculated value of the effect for the given channel
        StkFloat lastOut(unsigned int channel = 0);

        //! Provide one sample of mono or stereo import and return the given calculated channel
        StkFloat tick(StkFloat inputL, StkFloat inputR = 0.0, unsigned int channel = 0);

        //! Provide a frame of input (mono or stereo) and calculate stereo reverbed output with replacement
        StkFrames& tick(StkFrames& frames);

        //! Provide a frame of input (mono or stereo) and calculate stereo reverbed output without replacement
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
