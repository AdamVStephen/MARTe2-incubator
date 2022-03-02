#ifndef MFI_STMTIME_H_
#define MFI_STMTIME_H_

#include "CompilerTypes.h"

namespace MFI {
using namespace MARTe;

/**
 * Tracking of STM32 ticks, including wrap-around
 */
class StmTime {
 public:
    /**
     * Default out-of band value (like None in Python)
     */
    inline StmTime() : _cycles(-2) {}

    /**
     * Explicit out-of-band value
     */
    static inline StmTime invalid() { return StmTime(); }

    /**
     * Normal constructor
     */
    inline StmTime(int32 cycles, uint32 ticks) : _cycles(cycles), _ticks(ticks) {}

    /**
     * Constructor from tick value only
     */
    inline StmTime(uint32 ticks) : _cycles(0), _ticks(ticks) {}

    /**
     * Test for out-of-band value
     */
    inline bool is_valid() const { return _cycles != -2; }

    /**
     * Update from a new ticks value.
     * Assume the clock runs forward, but may roll over to a new cycle
     */
    bool tick_update(uint32 ticks);

    /**
     * Calculate number of ticks since another event.
     * Assumes the events are sufficiently close that the result fits in int32.
     */
    int32 ticks_since(const StmTime &other) const;

    /**
     * Infer time of one event, given only ticks value, from a nearby event.
     * Assumes that the time difference is less than a half cycle, either way.
     */
    StmTime nearby(uint32 ticks) const;

    /**
     * Return the number of full cycles
     */
    inline uint32 cycles() const { return _cycles; }

    /**
     * Return then number of ticks within the cycle
     */
    inline uint32 ticks() const { return _ticks; }

private:
    int32 _cycles;
    uint32 _ticks;
};

}
#endif /* MFI_STMTIME_H_ */
