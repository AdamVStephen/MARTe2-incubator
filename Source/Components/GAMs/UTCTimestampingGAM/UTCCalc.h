#ifndef MFI_UTCCALC_H_
#define MFI_UTCCALC_H_

#include "GeneralDefinitions.h"
#include <time.h>
#include "StmTime.h"
#include "UtcTime.h"
#include "LocalTime.h"

namespace MFI {

class UTCCalc {
public:
    UTCCalc();
    void time_check(LocalTime time);
    void gps_event(LocalTime time, uint32 week, uint32 tow);
    void adc_event(LocalTime time, uint32 adctime, uint32 ppstime);
    bool have_utc(UtcTime &time);
private:
    LocalTime m_localtime;
    float64 m_stm_freq;                 // Esimated STM tick frequency
    // from GPS data
    bool m_gps_ok;
    LocalTime m_gps_due;                // When next GPS msg expected
    UtcTime m_next_pps_utc;             // UTC at next rising edge
    // from STM32 ADC packet
    bool m_stm_ok;
    LocalTime m_stm_due;
    bool m_have_last_pps_tick;
    uint32 m_last_pps_tick;     // STM32 ticks at last PPS change
    StmTime m_adc_stime;        // Tracked STM time of last ADC sample
    // calculation
    UtcTime m_pps_utc;                  // UTC at last PPS edge
    StmTime m_pps_stime;                // STM32 time at last PPS edge
    // output
    bool m_adc_ok;
    UtcTime m_adc_utc;
};

}
#endif /* MFI_UTCCALC_H_ */
