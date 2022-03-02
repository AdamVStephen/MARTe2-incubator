#include "RaspPiSerial.h"
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

static void SerialThreadFunction(RaspPiSerial &serial);

RaspPiSerial::RaspPiSerial() : DataSourceI() {
    synchSem.Create();
    synchSem.Reset();
    buffer = NULL_PTR(char8*);
    message_size = 0;
    buffer_size = 0;
    serial_fd = 0;
    baud_rate = 0;
}

RaspPiSerial::~RaspPiSerial() {
    if (Threads::IsAlive(thread_id)) {
        Threads::Kill(thread_id);
    }

    if (buffer != NULL_PTR(char8*)) {
        delete [] buffer;
    }
}

bool RaspPiSerial::Initialise(MARTe::StructuredDataI& data) {
    bool ok = DataSourceI::Initialise(data);

    if (ok) {
        ok = data.Read("Port", port);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "No serial port has been specified");
        }
    }

    if (ok) {
        ok = data.Read("MessageSize", message_size);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "No message size has been specified");
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
        serial_fd = open(port.Buffer(), O_RDWR | O_NONBLOCK | O_NDELAY );
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

bool RaspPiSerial::Synchronise() {
    synchSem.ResetWait(TTInfiniteWait);

    return true;
}

bool RaspPiSerial::AllocateMemory() {
    return true;
}

bool RaspPiSerial::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx,
                                                 void *&signalAddress) {
    if (signalIdx == 0) {
        signalAddress = reinterpret_cast<void *>(buffer);

        return true;
    } else {
        signalAddress = NULL;

        return false;
    }
}

const MARTe::char8 *RaspPiSerial::GetBrokerName(StructuredDataI &data,
                                                        const SignalDirection direction) {
    if (direction == InputSignals) {
        return "MemoryMapSynchronisedInputBroker";
    }
    
    return NULL;
}

bool RaspPiSerial::GetInputBrokers(ReferenceContainer &inputBrokers,
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

bool RaspPiSerial::GetOutputBrokers(ReferenceContainer &outputBrokers,
                                            const char8* const functionName,
                                            void * const gamMemPtr) {
    return false;
}

bool RaspPiSerial::SetConfiguredDatabase(StructuredDataI & data) {
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    
    if (ok) {
        ok = (GetNumberOfSignals() == 1);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Only one signal can be specified");
        }
    }
    
    if (ok) {
        ok = (GetSignalType(0) == TypeDescriptor::GetTypeDescriptorFromTypeName("char8"));
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "The signal type has to be char8");
        }
    }

    if (ok) {
        uint8 numberOfDimensions;
        ok = GetSignalNumberOfDimensions(0u, numberOfDimensions);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Unable to get signal number of dimensions");
        } else {            
            ok = (numberOfDimensions == 1u);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "The signal must be of dimension 1");
            }
        }

        uint32 numberOfElements;
        if (ok) {
            ok = GetSignalNumberOfElements(0u, numberOfElements);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Unable to get signal number of elements");
            } else {            
                buffer_size = numberOfElements;
                ok = (buffer_size > message_size);
                if (!ok) {
                    REPORT_ERROR(ErrorManagement::InitialisationError, "The signal must be at least of size %u (MessageSize + 1)", message_size + 1);
                }
                else {
                    buffer = new char8[buffer_size];
                }
            }
        }
    }
    
    if (ok) {
        thread_id = Threads::BeginThread((ThreadFunctionType) SerialThreadFunction, this);
    }

    return true;
}

bool RaspPiSerial::PrepareNextState(const char8 * const currentStateName,
                                    const char8 * const nextStateName) {
    return true;
}

CLASS_REGISTER(RaspPiSerial, "1.0");

static void SerialThreadFunction(RaspPiSerial &serial) {
    
    uint32 chars_received = 0;
    
    while (1) {
        Sleep::Sec(1);

        int n = read(serial.serial_fd, 
                     serial.buffer + chars_received, 
                     serial.message_size - chars_received);
        
        if (n < 0) {
            // Error: decide what to do here
        } 
        else {
            chars_received += static_cast<uint32>(n);

            if (chars_received == serial.message_size) {
                memset(serial.buffer + serial.message_size, '\0', 
                       serial.buffer_size  - serial.message_size);
                chars_received = 0; 
                // NB: there is an assumption here that the signal will be processed before any
                // more characters can be received and potentially overwrite the buffer - need to 
                // improve this (maybe separate buffers?)
                serial.synchSem.Post();
            }
        } 
    }
}

} // namespace MFI
