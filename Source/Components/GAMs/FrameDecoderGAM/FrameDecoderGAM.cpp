#include "FrameDecoderGAM.h"
#include "AdvancedErrorManagement.h"
#include "GAMSignalChecker.h"

using namespace MARTe;

namespace MFI {

FrameDecoderGAM::FrameDecoderGAM() : GAM() {
    rx_data_frame = NULL_PTR(uint8*);
    adc_time = NULL_PTR(uint32*);
    pps1_time = NULL_PTR(uint32*);
    pps2_time = NULL_PTR(uint32*);
    pps2_time = NULL_PTR(uint32*);
    adc1_value = NULL_PTR(uint16*);
    adc2_value = NULL_PTR(uint16*);
}

FrameDecoderGAM::~FrameDecoderGAM() {

}

bool FrameDecoderGAM::Initialise(StructuredDataI & data) {
    return true;
}

bool FrameDecoderGAM::Setup() {
    StreamString gam_name;
    
    bool ok = GetQualifiedName(gam_name);
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "Cannot get the qualified name");
    }

    if (ok) {
        uint32 nOfInputSignals = GetNumberOfInputSignals();
        ok = (nOfInputSignals == 1u); // To do: finalise number of input signals
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "%s::Number of input signals must be 1", gam_name.Buffer());
        }
    } 
    
    if (ok) {
        uint32 signalIdx;
        ok = GAMCheckSignalProperties(*this, "RxDataFrame", InputSignals, UnsignedInteger8Bit, 1u, 32u, signalIdx);
        //ok = GAMCheckSignalProperties("RxDataFrame", InputSignals, UnsignedInteger8Bit, 1u, 32u, gam_name, signalIdx);
        if (ok) {
            rx_data_frame = (uint8*) GetInputSignalMemory(signalIdx);
        }
    }
    
    if (ok) {
        uint32 nOfOutputSignals = GetNumberOfOutputSignals();
        ok = (nOfOutputSignals == 5u); // To do: finalise number of output signals
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "%s::Number of output signals must be 5", gam_name.Buffer());
        }
    }
    
    if (ok) {
        uint32 signalIdx;
        
        ok = GAMCheckSignalProperties(*this, "ADCTime", OutputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            adc_time = (uint32*) GetInputSignalMemory(signalIdx);
        }
        ok = GAMCheckSignalProperties(*this, "PPS1Time", OutputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            pps1_time = (uint32*) GetInputSignalMemory(signalIdx);
        }
        ok = GAMCheckSignalProperties(*this, "PPS2Time", OutputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            pps2_time = (uint32*) GetInputSignalMemory(signalIdx);
        }
        ok = GAMCheckSignalProperties(*this, "ADC1Value", OutputSignals, UnsignedInteger16Bit, 0u, 1u, signalIdx);
        if (ok) {
            adc1_value = (uint16*) GetInputSignalMemory(signalIdx);
        }
        ok = GAMCheckSignalProperties(*this, "ADC2Value", OutputSignals, UnsignedInteger16Bit, 0u, 1u, signalIdx);
        if (ok) {
            adc2_value = (uint16*) GetInputSignalMemory(signalIdx);
        }
    }

    return ok;
}

bool FrameDecoderGAM::Execute() {
    *adc_time =  (uint32) rx_data_frame[0] + 
                ((uint32) rx_data_frame[1] << 8) + 
                ((uint32) rx_data_frame[2] << 16) + 
                ((uint32) rx_data_frame[3] << 24);
    
    *pps1_time = (uint32) rx_data_frame[4] + 
                ((uint32) rx_data_frame[5] << 8) + 
                ((uint32) rx_data_frame[6] << 16) + 
                ((uint32) rx_data_frame[7] << 24);

    *pps2_time = (uint32) rx_data_frame[8] + 
                ((uint32) rx_data_frame[9] << 8) + 
                ((uint32) rx_data_frame[10] << 16) + 
                ((uint32) rx_data_frame[11] << 24);
    
    *adc1_value = (uint16) rx_data_frame[12] + 
                 ((uint16) rx_data_frame[13] << 8); 

    *adc2_value = (uint16) rx_data_frame[12] + 
                 ((uint16) rx_data_frame[13] << 8);


    return true;
}

CLASS_REGISTER(FrameDecoderGAM, "1.0");

}  // namespace MFI
