#include "UTCCalc.h"
#include "AdvancedErrorManagement.h"

#ifndef INFO
#define WARN0(s) REPORT_ERROR_STATIC(ErrorManagement::Warning, s)
#define WARN(f,...) REPORT_ERROR_STATIC(ErrorManagement::Warning, f, __VA_ARGS__)
#define INFO0(s) REPORT_ERROR_STATIC(ErrorManagement::Information, s)
#define INFO(f,...) REPORT_ERROR_STATIC(ErrorManagement::Information, f, __VA_ARGS__)
#define DEBUG0(s) REPORT_ERROR_STATIC(ErrorManagement::Debug, s)
#define DEBUG(f,...) REPORT_ERROR_STATIC(ErrorManagement::Debug, f, __VA_ARGS__)
#endif

namespace MFI {

UTCCalc::UTCCalc() :
    m_stm_freq(10e6),
    m_gps_ok(false),
    m_next_pps_utc(),
    m_stm_ok(false),
    m_have_last_pps_tick(false),
    m_adc_stime(),
    m_pps_utc(),
    m_adc_ok(false)
{
}

/**
 * Note that time has passed
 * Set inputs invalid if too long since last event
 */
void UTCCalc::time_check(LocalTime time)
{
    m_localtime = time;
    if (m_gps_ok && time > m_gps_due) {
        WARN0("Lost GPS data");
        m_gps_ok = false;
    }
    if (m_stm_ok && time > m_stm_due) {
        WARN0("Lost STM32 data");
        m_stm_ok = false;
    }
}

/**
 * Consider a GPS message indicating week/time-of-week at the
 * next PPS rising edge.
 */
void UTCCalc::gps_event(LocalTime time, uint32 week, uint32 tow)
{
    if (! m_gps_ok) {
        INFO0("Receiving GPS data");
        m_gps_ok = true;
    }
    m_gps_due = time.plus_seconds(1.1); // 1 second plus leeway
    //GpsTime gps(week, tow);
    m_next_pps_utc = UtcTime::from_gps(week, tow);
}

/**
 * Consider an ADC data packet containing microcontroller
 * timestamps of ADC sample and last PPS rising edge,
 * plus ADC sample.
 */
void UTCCalc::adc_event(LocalTime time, uint32 adctick, uint32 ppstick)
{
    if (! m_stm_ok) {
        INFO0("Receiving packets from STM32");
        m_stm_ok = true;
        m_adc_stime = StmTime::invalid();
        m_have_last_pps_tick = false;
    }
    m_stm_due = time.plus_seconds(1.5e-4);   // Next expected at 10kHz plus leeway

    if (! m_adc_stime.is_valid()) {
        // Start to track.  Cycles immaterial until PPS seen
        m_adc_stime = StmTime(0, adctick);
    } else {
        // Track complete tick cycles as seen by ADC
        bool rolled = m_adc_stime.tick_update(adctick);
        if (rolled) {
            INFO0("Rollover");
        }
    }
    //DEBUG("adc_stime %s", m_adc_stime);

    // Until we see a change in PPS tick, we don't know times
    if (! m_have_last_pps_tick) {
        // Record first seen time, then wait for change
        m_last_pps_tick = ppstick;
        m_have_last_pps_tick = true;
    } else if (ppstick != m_last_pps_tick) {
        // STM32 appears to have seen a PPS rising edge.
        // Expect ppstick-0.1ms <= adctick <= ppstick+0.1ms
        // however there may be a tick rollover between
        DEBUG("PPS tick changed to %u", ppstick);
        StmTime pps_stime = m_adc_stime.nearby(ppstick);
        int32 ppsdiff = pps_stime.ticks_since(m_adc_stime);
        if (! (-1100 <= ppsdiff && ppsdiff <= 1100)) {
            // ? ADC and PPS not using same tick source?
            WARN("PPS tick change %u:%u misaligned with ADC tick %u:%u diff=%u",
                 pps_stime.cycles(), pps_stime.ticks(),
                 m_adc_stime.cycles(), m_adc_stime.ticks(),
                 ppsdiff);
        } else {
            // Looks like a plausible PPS timstamp.
            // If we have GPS, we know when it was.
            if (! m_gps_ok) {
                DEBUG0("PPS but no GPS");
            } else if (! m_next_pps_utc.is_valid()) {
                DEBUG0("PPS but no UTC");
            }
            if (m_gps_ok && m_next_pps_utc.is_valid()) {
                if (! m_pps_utc.is_valid()) {
                    INFO0("Sync with GPS");
                }
                // Consume the predicted event
                UtcTime new_pps_utc = m_next_pps_utc;
                m_next_pps_utc = UtcTime::invalid();
                // If we had a previous synchronisation, we can work out
                // the STM tick frequency from the UTC interval
                if (m_pps_utc.is_valid()) {
                    float64 interval_secs = new_pps_utc.difference(m_pps_utc);
                    int32 interval_ticks = pps_stime.ticks_since(m_pps_stime);
                    INFO("secs=%g ticks=%d", interval_secs, interval_ticks);
                    m_stm_freq = interval_ticks / interval_secs;
                    INFO("stm_freq=%g", m_stm_freq);
                }
                m_pps_utc = new_pps_utc;
                // Establish a new extrapolation, based in rollover-epoch
                // of the PPS tick.
                m_pps_stime = StmTime(0, ppstick);
                m_adc_stime = m_pps_stime.nearby(adctick);
                //DEBUG("pps=%s adc=%s");
            }
        }
        m_last_pps_tick = ppstick;
    }
    if (m_pps_utc.is_valid()) {
        // We have UTC of last PPS tick, so can infer UTC of sample
        int32 ticks_since_pps = m_adc_stime.ticks_since(m_pps_stime);
        float64 sec_since_pps = ticks_since_pps / m_stm_freq;
        UtcTime utc(m_pps_utc + sec_since_pps);
        m_adc_utc = utc;
        m_adc_ok = true;
    } else {
        m_adc_ok = false;
    }
}

bool UTCCalc::have_utc(UtcTime &time)
{
    if (m_adc_ok) {
        time = m_adc_utc;
    }
    return m_adc_ok;
}

}
