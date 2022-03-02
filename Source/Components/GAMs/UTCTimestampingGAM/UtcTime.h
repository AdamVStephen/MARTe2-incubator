#ifndef MFI_UTCTIME_H_
#define MFI_UTCTIME_H_

#include "GeneralDefinitions.h"
#include <ctime>

using namespace MARTe;

namespace MFI {

/**
 * Representation of UTC time.
 * FIXME should have some indication of leap second
 */
class UtcTime {
public:
    /**
     * Default (out-of-band) value (c.f. None in Python)
     */
    inline UtcTime() : m_dayno(0) { }

    /**
     * Explicit out of band value
     */
    static inline UtcTime invalid() { return UtcTime(); }

    /**
     * Normal constructor from a unix time and offset in seconds
     */
    UtcTime(time_t time, float64 seconds = 0.);

    /**
     * Test for 'None' value
     */
    inline bool is_valid() const { return m_dayno != 0; }

    /**
     * Create from GPS time: week number and time-of-week.
     * For GPS time oricinating from U-Blox module, the time has already
     * had leap-second correction such that tow==0 at midnight,
     * but it's unclear what happens if a leap-second occurs within a
     * week; perhaps tow goes up to 604800999.
     */
    static UtcTime from_gps(uint32 week, uint32 tow);

    /**
     * Calculate difference in seconds between two UtcTimes.
     * Behaviour across a leap-second is undefined.
     */
    float64 difference(UtcTime &other) const;

    /**
     * Create new UtcTime by applying a number of seconds.
     * Assumes no leap-second.
     */
    UtcTime operator + (float64 seconds) const;

    /**
     * Apply an offset of a number of seconds.
     * Assumes no leap-second.
     */
    UtcTime & operator += (float64 seconds);

    /**
     * Return the day number from start of Unix epoch.
     */
    inline uint32 day() const { return m_dayno; }

    /**
     * Return the number of seconds since midnight
     */
    inline float64 seconds() const { return m_seconds.tv_sec + m_seconds.tv_nsec / 1e9; }

    /**
     * Return seconds since Unix epoch (ignoring leap seconds)
     */
    inline uint32 unix_seconds() const { return m_dayno * 86400 + m_seconds.tv_sec; }

    /**
     * Return microseconds past the last full second
     */
    inline uint32 microseconds() const { return m_seconds.tv_nsec / 1000; }

private:
    uint32 m_dayno;                 // Days from Unix epoch 1970-01-01
    struct timespec m_seconds;          // From midnight; [0..86401)
};

}
#endif /* MFI_UTCTIME_H_ */
