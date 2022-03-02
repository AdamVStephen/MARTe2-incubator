#ifndef MFI_DATASOURCES_GPS_H_
#define MFI_DATASOURCES_GPS_H_

#include "DataSourceI.h"
#include "EventSem.h"
#include "MutexSem.h"

#include "UBX.h"

namespace MFI {

/**
 * Size of the GPS buffers
 */
extern const MARTe::uint32 GPS_BUFSIZE;

/**
 * @brief Convenience structure for grouping signals
 */
struct GPSSignals {
    GPSSignals();
    MARTe::uint32 received_byte_count;
    MARTe::uint32 discarded_byte_count;
    MARTe::uint32 read_error_count;
    MARTe::uint32 valid_message_count;
    MARTe::uint32 invalid_message_count;
    MARTe::uint8 message_valid;
    UBX::TIMTPPayload tim_tp_payload;
};

/**
 * @brief A DataSource which interfaces to a UBLOX M8N GPS receiver module, via a serial port
 * 
 * @details The GPS DataSource performs a blocking read of a serial port. It waits until a fixed number
 * of characters have been received, and then emits the characters as a input signal.
 * 
 * The DataSource shall have the following configuration (the object name `GPS` is an example - 
 * it is arbitrary):
 * <pre>
 * +GPS = {
 *     Class = GPS
 *     Port = "/dev/ttyS0"
 *     BaudRate = 9600
 *     Signals = {
 *           ReceivedByteCount = {
 *               Type = uint32
 *           }
 *           DiscardedByteCount = {
 *               Type = uint32
 *           }
 *           ReadErrorCount = {
 *               Type = uint32
 *           }
 *           ValidMessageCount = {
 *               Type = uint32
 *           }
 *           InvalidMessageCount = {
 *               Type = uint32
 *           }
 *           MessageValid = {
 *               Type = uint8
 *           }
 *           TimeOfWeek = { 
 *               Type = uint32
 *           }
 *           TimeOfWeekSubMS = {
 *               Type = uint32
 *           }
 *           QErr = {
 *               Type = int32
 *           }
 *           Week = {
 *               Type = uint16
 *           }
 *           Flags = {
 *               Type = uint8
 *           }
 *           RefInfo = {
 *               Type = uint8
 *           }
 *       }
 * }
 * </pre>
 * 
 * The Port is the path to the serial port device within the filesystem. The BaudRate is the data
 * speed that the DataSource will configure. 
 * 
 * Note that any characteristics of the serial port which are not covered by the configuration 
 * above (e.g. parity, number of data bits) will retain whatever default settings are defined
 * for the serial port.
 * 
 * The signals produced by the DataSource have the following meanings:
 * 
 * - ReceivedByteCount: The total number of bytes received on the serial port
 * 
 * - DiscardedByteCount: The toal number of bytes discarded on the serial port. Bytes are discarded
 * if the DataSource cannot assign them to a UBX-TIM-TP message. This may be due to partial messages
 * in the serial port buffer when the DataSource starts execution, corrupted messages, or receiving
 * messages of other types.
 * 
 * - ReadErrorCount: The number of read errors received on reading the serial port. Read errors 
 * correspond to the read system call returning a negative number. 
 * 
 * - ValidMessageCount: The number of UBX-TIM-TP messages received.
 * 
 * - InvalidMessageCount: Count of UBX messages received which were not of type UBX-TIM-TP
 * 
 * - MessageValid: Flag indicating if the current message is valid: either 0 (invalid) or 1 (valid).
 * A message is valid from the time it is received, up and including to the first time the DataSource's
 * Synchronise function is called i.e the first time the signals are received by a GAM. From then
 * on, if no message is received before the next call to Synchronise, the  MessageValid flag will be
 * set to 0. From the point of view of a receiving GAM, the GPS signals produced by this DataSource
 * should only be used when MessageValid has value 1.
 * 
 * - The remaining signals correspond to the contents of the UBX-TIM-TP message (see the uBlox receiver
 * protocol description for more information: https://www.u-blox.com/sites/default/files/products/documents/u-blox8-M8_ReceiverDescrProtSpec_UBX-13003221.pdf)
 * 
 */
class GPS : public MARTe::DataSourceI {
 public:

    CLASS_REGISTER_DECLARATION();
    
    GPS();
    virtual ~GPS();

    virtual bool Initialise(MARTe::StructuredDataI& data);
    
    virtual bool Synchronise();

    virtual bool AllocateMemory();

    virtual bool GetSignalMemoryBuffer(const MARTe::uint32 signalIdx, const MARTe::uint32 bufferIdx,
                                       void *&signalAddress);

    virtual const MARTe::char8 *GetBrokerName(MARTe::StructuredDataI &data,
                                              const MARTe::SignalDirection direction);

    virtual bool GetInputBrokers(MARTe::ReferenceContainer &inputBrokers,
                                 const MARTe::char8* const functionName,
                                 void * const gamMemPtr);

    virtual bool GetOutputBrokers(MARTe::ReferenceContainer &outputBrokers,
                                  const MARTe::char8* const functionName,
                                  void * const gamMemPtr);
    
    virtual bool SetConfiguredDatabase(MARTe::StructuredDataI & data);

    virtual bool PrepareNextState(const MARTe::char8 * const currentStateName,
                                  const MARTe::char8 * const nextStateName);

 public:
    /**
     * Synchronisation mutex
     */
    MARTe::MutexSem mutexSem;

    /**
     * File descriptor for the serial port
     */
    int serial_fd;

    /**
     * @brief Shared signals with the serial thread
     */
    GPSSignals shared_signals;

private:
    
    /**
     * Serial port path
     */
    MARTe::StreamString port;

    /**
     * Baud rate of the serial port
     */
    MARTe::uint32 baud_rate;

    /**
     * Serial thread id
     */
    MARTe::ThreadIdentifier thread_id;
    
    /** 
     * Outbound signals provided by the DataSource
     */
    GPSSignals out_signals;

    /**
     * Copy of the last UBX-TIM-TP payload received
     */
    UBX::TIMTPPayload prev_payload;
};

}
#endif // MFI_DATASOURCES_GPS_H_