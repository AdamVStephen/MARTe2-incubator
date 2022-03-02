#include "FrameEncoderGAM.h"
#include "AdvancedErrorManagement.h"
#include "GAMSignalChecker.h"

using namespace MARTe;

namespace MFI {

FrameEncoderGAM::FrameEncoderGAM() : GAM() {
    tx_data_frame = NULL_PTR(uint8*);
    dac1_demand = NULL_PTR(uint16*);
    dac2_demand = NULL_PTR(uint16*);
}

FrameEncoderGAM::~FrameEncoderGAM() {

}

bool FrameEncoderGAM::Initialise(StructuredDataI & data) {
    return true;
}

bool FrameEncoderGAM::Setup() {
    StreamString gam_name;
    
    bool ok = GetQualifiedName(gam_name);
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "Cannot get the qualified name");
    }

    if (ok) {
        uint32 nOfInputSignals = GetNumberOfInputSignals();
        ok = (nOfInputSignals == 2u); // To do: finalise number of input signals
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "%s::Number of input signals must be 1", gam_name.Buffer());
        }
    } 
    
    if (ok) {
        uint32 signalIdx;
        ok = GAMCheckSignalProperties(*this, "DAC1Demand", InputSignals, UnsignedInteger16Bit, 0u, 1u, signalIdx);
        if (ok) {
            dac1_demand = (uint16*) GetInputSignalMemory(signalIdx);
        }
        ok = GAMCheckSignalProperties(*this, "DAC2Demand", InputSignals, UnsignedInteger16Bit, 0u, 1u, signalIdx);
        if (ok) {
            dac2_demand = (uint16*) GetInputSignalMemory(signalIdx);
        }
    }
    
    if (ok) {
        uint32 nOfOutputSignals = GetNumberOfOutputSignals();
        ok = (nOfOutputSignals == 1u); // To do: finalise number of output signals
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "%s::Number of output signals must be 1", gam_name.Buffer());
        }
    }
    
    if (ok) {
        uint32 signalIdx;
        
        ok = GAMCheckSignalProperties(*this, "TxDataFrame", OutputSignals, UnsignedInteger32Bit, 1u, 32u, signalIdx);
        if (ok) {
            tx_data_frame = (uint8*) GetInputSignalMemory(signalIdx);
        }
    }

    return ok;
}

bool FrameEncoderGAM::Execute() {
    // To do: add in encoding of a frame header
    uint8* ptr = (uint8*) dac1_demand;
    
    tx_data_frame[0] = ptr[0];
    tx_data_frame[1] = ptr[1];
    
    ptr = (uint8*) dac2_demand;

    tx_data_frame[2] = ptr[0];
    tx_data_frame[3] = ptr[1];
    
    return true;
}

CLASS_REGISTER(FrameEncoderGAM, "1.0");

}  // namespace MFI
