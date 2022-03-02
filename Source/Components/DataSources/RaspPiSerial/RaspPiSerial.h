#ifndef MFI_DATASOURCES_RASPPISERIAL_H_
#define MFI_DATASOURCES_RASPPISERIAL_H_

#include "DataSourceI.h"
#include "EventSem.h"

namespace MFI {

/**
 * @brief A DataSource which monitors for fixed-size messages received on a serial port
 * 
 * @details The RaspPiSerial performs a blocking read of a serial port. It waits until a fixed number
 * of characters have been received, and then emits the characters as a input signal.
 * 
 * The DataSource shall have the following configuration (the object name `Serial` is an example - 
 * it is arbitrary):
 * <pre>
 * +Serial = {
 *     Class = RaspPiSerial
 *     Port = "/dev/ttyS0"
 *     BaudRate = 9600
 *     MessageSize = 8
 *     Signals = {
 *         Buffer = {
 *             Type = char8
 *             NumberOfDimensions = 1
 *             NumberOfElements = 9
 *         }
 *     }
 * }
 * </pre>
 * 
 * The Port is the path to the serial port device within the filesystem. The BaudRate is the data
 * speed that the DataSource will configure. The MessageSize is the expected size of the serial 
 * messages: the DataSource will not emit a signal until that number of characters has been received.
 * 
 * There must be a single Signal defined, of type char8 and NumberOfDimensions 1. The 
 * NumberOfElements is not fixed, but must be at least of size MessageSize + 1. When the signal is
 * emitted, the first MessageSize characters of the signal will contain the message received; the 
 * remaining characters will be set to '\0'.
 * 
 * Note that any characteristics of the serial port which are not covered by the configuration 
 * above (e.g. parity, number of data bits) will retain whatever default settings are defined
 * for the serial port.
 */
class RaspPiSerial : public MARTe::DataSourceI {
 public:

    CLASS_REGISTER_DECLARATION();
    
    RaspPiSerial();
    virtual ~RaspPiSerial();

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
     * Synchronisation semaphore
     */
    MARTe::EventSem synchSem;

    /**
     * Serial port path
     */
    MARTe::StreamString port;

    /**
     * Message buffer
     */
    MARTe::char8* buffer;

    /**
     * Size of the received messages
     */
    MARTe::uint32 message_size;

    /**
     * Size of the message buffer
     */
    MARTe::uint32 buffer_size;

    /**
     * File descriptor for the serial port
     */
    int serial_fd;

    /**
     * Baud rate of the serial port
     */
    MARTe::uint32 baud_rate;

    /**
     * Serial thread id
     */
    MARTe::ThreadIdentifier thread_id;
};

} // namespace MFI

#endif // MFI_DATASOURCES_RASPPISERIAL_H_
