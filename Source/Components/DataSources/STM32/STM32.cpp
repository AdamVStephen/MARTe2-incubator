#include "STM32.h"
#include "DataSourceSignalChecker.h"
#include "AdvancedErrorManagement.h"
#include "MemoryMapSynchronisedInputBroker.h"
#include "Threads.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

using namespace MARTe;

namespace MFI {

ADCSignals::ADCSignals(): adc_time(0u),
                          pps1_time(0u),
                          pps2_time(0u),
                          adc1_data(0u),
                          adc2_data(0u) {

}

static void SerialThreadFunction(STM32 &serial);

STM32::STM32() : DataSourceI(), 
                 serial_fd(0),
                 port(""),
                 baud_rate(0),
                 thread_id(0),
                 out_signals() {
    synchSem.Create();
    synchSem.Reset();
}

STM32::~STM32() {
    if (Threads::IsAlive(thread_id)) {
        Threads::Kill(thread_id);
    }
}

bool STM32::Initialise(MARTe::StructuredDataI& data) {
    bool ok = DataSourceI::Initialise(data);

    if (ok) {
        ok = data.Read("Port", port);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "No serial port has been specified");
        }
    }

    if (ok) {
        ok = data.Read("BaudRate", baud_rate);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "No baud rate has been specified");
        }
    }

    // Open and configure the serial port
    if (ok) {
        serial_fd = open(port.Buffer(), O_RDWR);
        ok = !(serial_fd < 0);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Failed to open serial port");
        }

        struct termios tty;
        ok = (tcgetattr(serial_fd, &tty) == 0);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Failed to get serial port settings");
        }
        if (ok) {
            tty.c_ispeed = baud_rate;
            tty.c_ospeed = baud_rate;
            ok = (tcsetattr(serial_fd, TCSANOW, &tty) == 0);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Failed to set serial port settings");
            }
        }
    }

    return ok;
}

bool STM32::Synchronise() {
    synchSem.ResetWait(TTInfiniteWait);

    return true;
}

bool STM32::AllocateMemory() {
    return true;
}

bool STM32::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx,
                                  void *&signalAddress) {
    signalAddress = NULL;
    
    if (signalIdx == 0u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.adc_time);
    } else if (signalIdx == 1u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.pps1_time);
    } else if (signalIdx == 2u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.pps2_time);
    } else if (signalIdx == 3u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.adc1_data);
    } else if (signalIdx == 4u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.adc2_data);
    } else {
        ;
    }
    
    if (signalAddress == NULL) {
        return false;
    } else {
        return true;
    }
}

const MARTe::char8 *STM32::GetBrokerName(StructuredDataI &data,
                                         const SignalDirection direction) {
    if (direction == InputSignals) {
        return "MemoryMapSynchronisedInputBroker";
    }

    return NULL;
}

bool STM32::GetInputBrokers(ReferenceContainer &inputBrokers,
                            const char8* const functionName,
                            void * const gamMemPtr) {
    ReferenceT<MemoryMapSynchronisedInputBroker> broker("MemoryMapSynchronisedInputBroker");
    bool ret = broker.IsValid();
    if (ret) {
        ret = broker->Init(InputSignals, *this, functionName, gamMemPtr);
    }
    if (ret) {
        ret = inputBrokers.Insert(broker);
    }

    return ret;
}

bool STM32::GetOutputBrokers(ReferenceContainer &outputBrokers,
                                            const char8* const functionName,
                                            void * const gamMemPtr) {
    return false;
}

bool STM32::SetConfiguredDatabase(StructuredDataI & data) {
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    
    if (ok) {
        ok = (GetNumberOfSignals() == 5u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Exactly 5 signals should be specified");
        }
    }
    
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 0u, UnsignedInteger32Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ADCTime");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 1u, UnsignedInteger32Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal PPS1Time");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 2u, UnsignedInteger32Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal PPS2Time");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 3u, UnsignedInteger16Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ADC1Data");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 4u, UnsignedInteger16Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ADC2Data");
        }
    }
    
    if (ok) {
        thread_id = Threads::BeginThread((ThreadFunctionType) SerialThreadFunction, this);
    }

    return ok;
}

bool STM32::PrepareNextState(const char8 * const currentStateName,
                                    const char8 * const nextStateName) {
    return true;
}

CLASS_REGISTER(STM32, "1.0");

static void SerialThreadFunction(STM32 &serial) {
    uint8 buffer[16];

    while (1) {
        int n = read(serial.serial_fd,  buffer, 16);        
        uint32 chars_received = static_cast<uint32>(n);

        if (chars_received > 0) {
            serial.synchSem.Post();
        }
    }
}

} // namespace MFI
