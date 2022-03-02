#include "UTCTimestampingGAM.h"
#include "AdvancedErrorManagement.h"
#include "GAMSignalChecker.h"

using namespace MARTe;

namespace MFI {

UTCTimestampingGAM::UTCTimestampingGAM() : GAM() {
    nominal_stm_freq = 0u;
    in_adc_time = NULL_PTR(uint32*);
    in_pps1_time = NULL_PTR(uint32*);
    in_pps2_time = NULL_PTR(uint32*);
    in_adc1_data = NULL_PTR(uint16*);
    in_adc2_data = NULL_PTR(uint16*);
    in_gps_message_valid = NULL_PTR(uint8*);
    in_time_of_week = NULL_PTR(uint32*);
    in_time_of_week_sub_ms = NULL_PTR(uint32*);
    in_q_err = NULL_PTR(int32*);
    in_week = NULL_PTR(uint16*);
    in_flags = NULL_PTR(uint8*);
    in_ref_info = NULL_PTR(uint8*);
    out_adc1_data = NULL_PTR(uint16*);
    out_adc2_data = NULL_PTR(uint16*);
    out_adc_utc_unix_seconds = NULL_PTR(uint32*);
    out_adc_utc_microseconds = NULL_PTR(uint32*);
    out_stm_freq = NULL_PTR(uint32*);
    out_validity = NULL_PTR(uint8*);
    out_time_since_last_pps_sync = NULL_PTR(uint32*);
}

UTCTimestampingGAM::~UTCTimestampingGAM() {

}

/**
 * Fetch any configuration values
 */
bool UTCTimestampingGAM::Initialise(StructuredDataI & data) {
    bool ok = GAM::Initialise(data);

    if (ok) {
        ok = data.Read("NominalSTMFreq", nominal_stm_freq);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "No nominal STM frequency has been specified");
        }
    }
    
    return ok;
}

/**
 * Get access to input and output signals
 */
bool UTCTimestampingGAM::Setup() {
    StreamString gam_name;
    
    bool ok = GetQualifiedName(gam_name);
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "Cannot get the qualified name");
    }

    if (ok) {
        uint32 nOfInputSignals = GetNumberOfInputSignals();
        ok = (nOfInputSignals == 12u); // Will need to be changed if any input signals are added or removed
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "%s::Number of input signals must be 12", gam_name.Buffer());
        }
    } 
    uint32 signalIdx;
    if (ok) {    
        ok = GAMCheckSignalProperties(*this, "ADCTime", InputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_adc_time = (uint32*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ADCTime");
        }
    }
    if (ok) {
        ok = GAMCheckSignalProperties(*this, "PPS1Time", InputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_pps1_time = (uint32*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal PPS1Time");
        }
    }
    if (ok) {
        ok = GAMCheckSignalProperties(*this, "PPS2Time", InputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_pps2_time = (uint32*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal PPS2Time");
        }
    }
    if (ok) {
        ok = GAMCheckSignalProperties(*this, "ADC1Data", InputSignals, UnsignedInteger16Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_adc1_data = (uint16*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ADC1Data");
        }
    }
    if (ok) {
        ok = GAMCheckSignalProperties(*this, "ADC2Data", InputSignals, UnsignedInteger16Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_adc2_data = (uint16*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ADC2Data");
        }
    }
    if (ok) {
        ok = GAMCheckSignalProperties(*this, "MessageValid", InputSignals, UnsignedInteger8Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_gps_message_valid = (uint8*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal MessageValid");
        }
    }
    if (ok) {
        ok = GAMCheckSignalProperties(*this, "TimeOfWeek", InputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_time_of_week = (uint32*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal TimeOfWeek");
        }
    }
    if (ok) {
        ok = GAMCheckSignalProperties(*this, "TimeOfWeekSubMS", InputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_time_of_week_sub_ms = (uint32*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal TimeOfWeekSubMS");
        }
    }
    if (ok) {
        ok = GAMCheckSignalProperties(*this, "QErr", InputSignals, SignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_q_err = (int32*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal QErr");
        }
    }
    if (ok) {
        ok = GAMCheckSignalProperties(*this, "Week", InputSignals, UnsignedInteger16Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_week = (uint16*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal Week");
        }
    }
    if (ok) {
        ok = GAMCheckSignalProperties(*this, "Flags", InputSignals, UnsignedInteger8Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_flags = (uint8*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal Flags");
        }
    }
    if (ok) {
        ok = GAMCheckSignalProperties(*this, "RefInfo", InputSignals, UnsignedInteger8Bit, 0u, 1u, signalIdx);
        if (ok) {
            in_ref_info = (uint8*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal RefInfo");
        }
    }

    if (ok) {
        uint32 nOfOutputSignals = GetNumberOfOutputSignals();
        ok = (nOfOutputSignals == 6u); // Will need to be changed if any output signals are added or removed
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "%s::Number of output signals must be 6", gam_name.Buffer());
        }
    }
    if (ok) {    
        ok = GAMCheckSignalProperties(*this, "ADC1Data", OutputSignals, UnsignedInteger16Bit, 0u, 1u, signalIdx);
        if (ok) {
            out_adc1_data = (uint16*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ADC1Data");
        }
    }
    if (ok) {    
        ok = GAMCheckSignalProperties(*this, "ADC2Data", OutputSignals, UnsignedInteger16Bit, 0u, 1u, signalIdx);
        if (ok) {
            out_adc2_data = (uint16*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ADC2Data");
        }
    }
    if (ok) {    
        ok = GAMCheckSignalProperties(*this, "ADCUTCUnixSeconds", OutputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            out_adc_utc_unix_seconds = (uint32*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ADCUTCUnixSeconds");
        }
    }
    if (ok) {    
        ok = GAMCheckSignalProperties(*this, "ADCUTCMicroseconds", OutputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            out_adc_utc_microseconds = (uint32*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ADCUTCMicroseconds");
        }
    }
    if (ok) {    
        ok = GAMCheckSignalProperties(*this, "STMFreq", OutputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            out_stm_freq = (uint32*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal STMFreq");
        }
    }
    if (ok) {    
        ok = GAMCheckSignalProperties(*this, "Validity", OutputSignals, UnsignedInteger8Bit, 0u, 1u, signalIdx);
        if (ok) {
            out_validity = (uint8*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal Validity");
        }
    }
    if (ok) {    
        ok = GAMCheckSignalProperties(*this, "TimeSinceLastPPSSync", OutputSignals, UnsignedInteger32Bit, 0u, 1u, signalIdx);
        if (ok) {
            out_time_since_last_pps_sync = (uint32*) GetInputSignalMemory(signalIdx);
        } else {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal TimeSinceLastPPSSync");
        }
    }

    return ok;
}

/**
 * Run the timestamping algorithm.
 * In normal operation, expect to be called whenever there is a new
 * packet of information (ADC samples + timestamps) from the STM32.
 *
 * FIXME: How do we behave if nothing is arriving?
 * FIXME: What if PPS data but no STM data?
 */
bool UTCTimestampingGAM::Execute() {
    // Get the local time.
    // This allows us to track arrival of expected inputs
    LocalTime time_now(HighResolutionTimer::Counter());

    // Check if a new GPS message has been seen
    if (! *in_gps_message_valid) {
        last_gps_week = 0;              // Indicates invalid
        last_gps_tow = 0;
    } else {
        uint32 week = *in_week;
        uint32 tow = *in_time_of_week;
        if (! (week == last_gps_week && tow == last_gps_tow)) {
            // Handle new UTC of next PPS
            calc.gps_event(time_now, week, tow);

            last_gps_week = week;
            last_gps_tow = tow;
        }
        // FIXME Any use for q_err, flags or ref_info?
    }

    // See if any events passed (e.g. missing GPS)
    calc.time_check(time_now);

    // Now handle the STM32 packet
    uint32 adc_tick = *in_adc_time;
    uint32 pps_tick = *in_pps1_time;
    // FIXME pps2
    uint16 adc1_val = *in_adc1_data;
    uint16 adc2_val = *in_adc1_data;
    calc.adc_event(time_now, adc_tick, pps_tick);

    // See if we have a UTC timestamp for these samples
    UtcTime utc;
    if (calc.have_utc(utc)) {
        *out_adc1_data = adc1_val;
        *out_adc2_data = adc2_val;
        *out_adc_utc_unix_seconds = utc.unix_seconds();
        *out_adc_utc_microseconds = utc.microseconds();
        *out_validity = 1;              // FIXME multi-state?
    } else {
        *out_validity = 0;
    }

    // FIXME out_time_since_last_pps_sync

    return true;
}

CLASS_REGISTER(UTCTimestampingGAM, "1.0");

} // namespace MFI
