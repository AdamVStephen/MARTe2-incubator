#include "UtcTime.h"

namespace MFI {

UtcTime::UtcTime(time_t time, float64 seconds)
{
    m_dayno = time / 86400;
    m_seconds.tv_sec = time % 86400;
    m_seconds.tv_nsec = 0;
    *this += seconds;
}

UtcTime UtcTime::from_gps(uint32 week, uint32 tow)
{
    uint32 day = 3657 + week * 7;
    float64 sec = tow / 1e3;
    return UtcTime(day * 86400, sec);
}

float64 UtcTime::difference(UtcTime &other) const
{
    float64 sec = (m_dayno - other.m_dayno) * 86400.; // FIXME leap second
    sec += ((float64)m_seconds.tv_sec - (float64)other.m_seconds.tv_sec);
    // tv_sec is signed in range [0,999999999] so no subtraction overflow
    sec += (m_seconds.tv_nsec - other.m_seconds.tv_nsec) * 1e-9;
    return sec;
}

UtcTime UtcTime::operator + (float64 delta) const
{
    UtcTime out = *this;
    out += delta;
    return out;
}

UtcTime & UtcTime::operator += (float64 seconds)
{
    uint32 whole = (uint32) seconds;
    m_seconds.tv_sec += whole;
    m_seconds.tv_nsec += (seconds - whole) * 1e9;
    if (m_seconds.tv_nsec >= 1000000000) {
        m_seconds.tv_nsec -= 1000000000;
        m_seconds.tv_sec += 1;
    }
    // if (m_seconds.tv_sec >= 86400) { // FIXME leap seconds
    //     m_seconds.tv_sec -= 86400;
    //     m_day += 1;
    // }
    return *this;
}

}
