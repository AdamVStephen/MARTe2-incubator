#ifndef MFI_UTC_TIMESTAMPING_H_
#define MFI_UTC_TIMESTAMPING_H_

#include "CompilerTypes.h"
#include "GAM.h"
#include "UTCCalc.h"
#include "StmTime.h"
#include "UtcTime.h"
#include "LocalTime.h"

using namespace MARTe;

namespace MFI {

class UTCTimestampingGAM : public MARTe::GAM {
 public:
    
    CLASS_REGISTER_DECLARATION();
    
    UTCTimestampingGAM();
    virtual ~UTCTimestampingGAM();

    virtual bool Initialise(StructuredDataI & data);

    virtual bool Setup();

    virtual bool Execute();

 private:

    // Pointers to input signals as set by input broker

    // From STM32 data packet
    uint32* in_adc_time;         // Timestamp of ADC sample (STM32 ticks)
    uint32* in_pps1_time;        // Timestamp of PPS1 edge (STM32 ticks)
    uint32* in_pps2_time;        // Timestamp of PPS2 edge (STM32 ticks)
    uint16* in_adc1_data;        // Raw ADC channel 1 sample (0..4095)
    uint16* in_adc2_data;        // Raw ADC channel 2 sample (0..4095)

    // From GPS module
    uint8* in_gps_message_valid;
    uint32* in_time_of_week;     // Milliseconds from start of GPS week
    uint32* in_time_of_week_sub_ms;
    int32* in_q_err;             // Quantisation error (picoseconds)
    uint16* in_week;             // GPS week number (0 = w/c 1980-01-06)
    uint8* in_flags;
    uint8* in_ref_info;

    // Pointers to output signals, to be used by output broker
    uint16* out_adc1_data; 	// Raw ADC channel 1 sample
    uint16* out_adc2_data; 	// Raw ADC channel 1 sample
    uint32* out_adc_utc_unix_seconds;  // UTC timestamp of ADC samples (sec)
    uint32* out_adc_utc_microseconds;  // UTC timestamp of ADC samples (usec)
    uint32* out_stm_freq;        // Inferred STM tick frequency (Hz)
    uint8* out_validity;

    uint32* out_time_since_last_pps_sync;

    // Configuration values
    uint32 nominal_stm_freq;     // Expected tick frequency (e.g. 10MHz)

    // Internal state
    uint32 last_gps_week;
    uint32 last_gps_tow;
    UTCCalc calc;
};

} // namespace MFI

#endif // MFI_UTC_TIMESTAMPING_H_
