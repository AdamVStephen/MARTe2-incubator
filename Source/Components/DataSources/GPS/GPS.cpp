#include "GPS.h"
#include "GPSBuffer.h"
#include "UBX.h"
#include "DataSourceSignalChecker.h"
#include "AdvancedErrorManagement.h"
#include "MemoryMapSynchronisedInputBroker.h"
#include "Threads.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/serial.h>

using namespace MARTe;

namespace MFI {

const MARTe::uint32 GPS_BUFSIZE = 256;

static void SerialThreadFunction(GPS &gps);

GPSSignals::GPSSignals() :
    received_byte_count(0u),
    discarded_byte_count(0u),
    read_error_count(0u),
    valid_message_count(0u),
    invalid_message_count(0u),
    message_valid(0u),
    tim_tp_payload() {}

GPS::GPS() : DataSourceI(), 
             serial_fd(0),
             shared_signals(),
             port(""),
             baud_rate(0),
             thread_id(0u),
             out_signals(),
             prev_payload() {
    mutexSem.Create();
}

GPS::~GPS() {
    if (Threads::IsAlive(thread_id)) {
        Threads::Kill(thread_id);
    }

    mutexSem.Close();
}

bool GPS::Initialise(MARTe::StructuredDataI& data) {
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
    if (ok) {;
        serial_fd = open(port.Buffer(), O_RDWR);
        ok = !(serial_fd < 0);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Failed to open serial port %s", port.Buffer());
        }
    }

    if (ok) {
        struct termios tty;
        ok = (tcgetattr(serial_fd, &tty) == 0);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Failed to get settings of serial port %s", port.Buffer());
        }
        if (ok) {
            tty.c_ispeed = baud_rate;
            tty.c_ospeed = baud_rate;
            cfmakeraw(&tty);
            tty.c_cflag &= ~CSTOPB;
            tty.c_cflag &= ~CRTSCTS;
            tty.c_cflag |= CLOCAL | CREAD;
            
            ok = (tcsetattr(serial_fd, TCSANOW, &tty) == 0);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Failed to apply settings of serial port %s", port.Buffer());
            }
        }
    }

    return ok;
}

bool GPS::Synchronise() {   
    mutexSem.Lock();
    out_signals = shared_signals;
    mutexSem.UnLock();

    // Message is only valid if it has changed (or if we haven't received any messages yet)
    if (out_signals.valid_message_count == 0 || out_signals.tim_tp_payload == prev_payload) {
        out_signals.message_valid = 0u;
    } else {
        out_signals.message_valid = 1u;
    }
    prev_payload = out_signals.tim_tp_payload;

    return true;
}

bool GPS::AllocateMemory() {
    return true;
}

bool GPS::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx,
                                                 void *&signalAddress) {
    signalAddress = NULL;

    if (signalIdx == 0u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.received_byte_count);
    } else if (signalIdx == 1u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.discarded_byte_count);
    } else if (signalIdx == 2u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.read_error_count);
    } else if (signalIdx == 3u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.valid_message_count);
    } else if (signalIdx == 4u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.invalid_message_count);
    } else if (signalIdx == 5u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.message_valid);
    } else if (signalIdx == 6u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.tim_tp_payload.tow_ms);
    } else if (signalIdx == 7u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.tim_tp_payload.tow_sub_ms);
    } else if (signalIdx == 8u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.tim_tp_payload.q_err);
    } else if (signalIdx == 9u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.tim_tp_payload.week);
    } else if (signalIdx == 10u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.tim_tp_payload.flags);
    } else if (signalIdx == 11u) {
        signalAddress = reinterpret_cast<void *>(&out_signals.tim_tp_payload.ref_info);
    } else {
        ;
    }

    if (signalAddress == NULL) {
        return false;
    } else {
        return true;
    }
}

const MARTe::char8 *GPS::GetBrokerName(StructuredDataI &data,
                                       const SignalDirection direction) {
    if (direction == InputSignals) {
        return "MemoryMapSynchronisedInputBroker";
    }
    
    return NULL;
}

bool GPS::GetInputBrokers(ReferenceContainer &inputBrokers,
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

bool GPS::GetOutputBrokers(ReferenceContainer &outputBrokers,
                                            const char8* const functionName,
                                            void * const gamMemPtr) {
    return false;
}

bool GPS::SetConfiguredDatabase(StructuredDataI & data) {
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    
    if (ok) {
        ok = (GetNumberOfSignals() == 12u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Exactly 9 signals should be specified");
        }
    }

    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 0u, UnsignedInteger32Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ReceivedByteCount");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 1u, UnsignedInteger32Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal DiscardedByteCount");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 2u, UnsignedInteger32Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ReadErrorCount");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 3u, UnsignedInteger32Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal ValidMessageCount");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 4u, UnsignedInteger32Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal InvalidMessageCount");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 5u, UnsignedInteger8Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal MessageValid");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 6u, UnsignedInteger32Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal TimeOfWeek");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 7u, UnsignedInteger32Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal TimeOfWeekSubMS");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 8u, SignedInteger32Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal QErr");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 9u, UnsignedInteger16Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal Week");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 10u, UnsignedInteger8Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal Flags");
        }
    }
    if (ok) {
        ok = DataSourceCheckSignalProperties(*this, 11u, UnsignedInteger8Bit, 0u, 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Signal properties check failed for signal RefInfo");
        }
    }

    if (ok) {
        thread_id = Threads::BeginThread((ThreadFunctionType) SerialThreadFunction, this);
    }

    return true;
}

bool GPS::PrepareNextState(const char8 * const currentStateName,
                           const char8 * const nextStateName) {
    return true;
}

CLASS_REGISTER(GPS, "1.0");

static void SerialThreadFunction(GPS &gps) {
    GPSBuffer buffer(256);
    GPSSignals signals;

    while (1) {           
        uint8 temp_buffer[256];
        
        // Blocking read
        int ret = read(gps.serial_fd, temp_buffer, sizeof(temp_buffer));

        if (ret < 0) {
            // To do: fix problem where the transfer of the updated signals is skipped by this continue
            signals.read_error_count++;
            continue;
        }
        
        uint32 nbytes = static_cast<uint32>(ret);
        signals.received_byte_count += nbytes;
        
        uint32 nbytes_queued = buffer.queue(temp_buffer, nbytes);
        signals.discarded_byte_count += (nbytes - nbytes_queued);

        // The buffer could contain at most BUFSIZE / UBX::TIM_TP_MSG_SIZE messages for processing.
        // In principle there should be no more than one message available, since we process received 
        // bytes immediately. However, if the thread were starved of CPU time, this would not be the
        // case, and we would need to process multiple buffered messages to catch up - hence this loop
        for (uint32 msg = 0; msg < GPS_BUFSIZE / UBX::TIM_TP_MSG_SIZE; msg++) {
            // Sanitise the buffer - delete any leading rogue bytes that cannot be part of a UBX message
            uint32 sanitise_discard = UBX::SanitiseBuffer(buffer);
            signals.discarded_byte_count += sanitise_discard;

            uint32 get_messages_discard = 0u;
            UBX::Message message;
            bool ok = UBX::GetNextMessage(buffer, message, get_messages_discard);
            // If getting a message has failed, record any discarded bytes
            signals.discarded_byte_count += get_messages_discard;

            // If we've found a valid UBX message
            if (ok) {
                // Check that we have a UBX-TIM-TP message
                if (message.msg_class == UBX::TIMTPMessageClass && 
                        message.msg_id == UBX::TIMTPMessageId && 
                        message.payload_len == UBX::TIMTPPayloadLength) {   
                    UBX::ParseTIMTPPayload(message.payload, signals.tim_tp_payload);
                    signals.valid_message_count++;
                } else {
                    // If not a UBX-TIM-TP, discard the message
                    signals.invalid_message_count++;
                    signals.discarded_byte_count += (message.payload_len + 8);
                }
            } else {
                // If there's no message available we can exit the loop immediately
                break;
            }
        }
        
        // Transfer the updated signals back to the DataSource
        gps.mutexSem.Lock();
        gps.shared_signals = signals;     
        gps.mutexSem.UnLock();
    }
}

} // namespace MFI
