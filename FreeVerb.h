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

        void clear();

        StkFloat lastOut(unsigned int channel = 0);

        StkFloat tick(StkFloat input, unsigned int channel = 0);

        StkFrames& tick(StkFrames& frames, unsigned int channel = 0);

        StkFrames& tick(StkFrames& iFrames, StkFrames &oFrames, unsigned int iChannel = 0, unsigned int oChannel =0);

    protected:
        int numComb_, numAllPass_;
        StkFloat feedback_; // feedback coefficient
        StkFloat g_;        // allpass coefficient

        // LBFC: Lowpass Feedback Comb Filters
        Delay combDelayL_[8];
        Delay combDelayR_[8];
        OnePole combFilterL_[8];
        OnePole combFilterR_[8];
        
        // AP: Allpass Filters
        Delay allPassDelayL_[4];
        Delay allPassDelayR_[4];
};

}

#endif
