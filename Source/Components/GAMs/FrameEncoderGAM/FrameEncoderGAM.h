#ifndef FRAME_ENCODER_GAM_H_
#define FRAME_ENCODER_GAM_H_

#include "GAM.h"

namespace MFI {

class FrameEncoderGAM: public MARTe::GAM {
 public:
 
    CLASS_REGISTER_DECLARATION();

    FrameEncoderGAM();

    virtual ~FrameEncoderGAM();

    virtual bool Initialise(MARTe::StructuredDataI & data);

    virtual bool Setup();

    virtual bool Execute();

 private:
    
    MARTe::uint8* tx_data_frame;

    MARTe::uint16* dac1_demand;

    MARTe::uint16* dac2_demand;
};

}  // namespace MFI

#endif  // FRAME_ENCODER_GAM_H_
