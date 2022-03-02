#include "StmTime.h"

#define STM_TICK_CYCLE 3000000000
#define STM_TICK_HALFCYCLE (STM_TICK_CYCLE / 2)

namespace MFI {

/**
 * Update to new ticks value, inferring cycles
 */
bool StmTime::tick_update(uint32 ticks) {
    bool rollover = false;
    if (ticks < _ticks) {
        _cycles += 1;
        rollover = true;
    }
    _ticks = ticks;
    return rollover;
}

/**
 * Calculate signed difference between two times, in ticks.
 * Assume difference is no more than INT32_MAX
 */
int32 StmTime::ticks_since(const StmTime &other) const {
    int32 diff = (int32)_ticks - (int32)other._ticks;
    diff += (int32)(_cycles - other._cycles) * STM_TICK_CYCLE;
    return diff;
}

/**
 * Given ticks only of an event, infer nearby time
 * and return signed tick difference
 */
StmTime StmTime::nearby(uint32 alt) const
{
    int32 altcycles;
    if (_ticks < STM_TICK_HALFCYCLE && alt >= STM_TICK_HALFCYCLE) {
        // e.g. cur=0000002 and alt=9999995
        // alt yet to roll over
        altcycles = _cycles - 1;
    } else if (_ticks >= STM_TICK_HALFCYCLE && alt < STM_TICK_HALFCYCLE) {
        // e.g. cur=9999994 and alt=0000007
        // alt has already rolled over
        altcycles = _cycles + 1;
    } else {
        // Both in the same half-cycle
        altcycles = _cycles;
    }
    return StmTime(altcycles, alt);
}

}
