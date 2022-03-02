#ifndef FRAME_DECODER_GAM_H_
#define FRAME_DECODER_GAM_H_

#include "GAM.h"

namespace MFI {

class FrameDecoderGAM: public MARTe::GAM {
 public:
 
    CLASS_REGISTER_DECLARATION();

    FrameDecoderGAM();

    virtual ~FrameDecoderGAM();

    virtual bool Initialise(MARTe::StructuredDataI & data);

    virtual bool Setup();

    virtual bool Execute();

 private:
    
    MARTe::uint8* rx_data_frame;

    MARTe::uint32* adc_time;

    MARTe::uint32* pps1_time;

    MARTe::uint32* pps2_time;

    MARTe::uint16* adc1_value;

    MARTe::uint16* adc2_value;
};

}  // namespace MFI

#endif  // FRAME_DECODER_GAM_H_
