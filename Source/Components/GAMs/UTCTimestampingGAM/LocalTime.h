#ifndef MFI_LOCALTIME_H_
#define MFI_LOCALTIME_H_

#include "GeneralDefinitions.h"

using namespace MARTe;

namespace MFI {

/**
 * Representation of local time in the RPi.
 * Assume based on MARTe::HighResolutionTimer
 */
class LocalTime {
public:
    uint64 ticks;
    inline LocalTime(uint64 t = 0) : ticks(t) {}
    inline operator float64() { return ticks * 1e-9; }
    inline LocalTime plus_seconds(float64 inc) { return LocalTime(ticks + static_cast<uint64>(inc * 1e9)); }
    inline bool operator > (LocalTime &other) { return ticks > other.ticks; }
};

}
#endif /* MFI_LOCALTIME_H_ */
