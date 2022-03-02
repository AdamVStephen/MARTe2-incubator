#ifndef MFI_DATASOURCES_STM32_H_
#define MFI_DATASOURCES_STM32_H_

#include "DataSourceI.h"
#include "EventSem.h"

namespace MFI {

struct ADCSignals {
    ADCSignals();
    MARTe::uint32 adc_time;
    MARTe::uint32 pps1_time;
    MARTe::uint32 pps2_time;
    MARTe::uint16 adc1_data;
    MARTe::uint16 adc2_data;
};

/**
 * @brief A DataSource for reception and tranmission of ADC and DAC data frames from and to the
 * STM32 serial interface
 * 
 * @details The STM32 performs a blocking read of a serial port. It waits until a fixed number
 * of characters have been received, and then emits the characters as a input signal.
 * 
 * The DataSource shall have the following configuration (the object name `STM32` is an example - 
 * it is arbitrary):
 * <pre>
 * +STM32 = {
 *     Class = STM32
 *     Port = "/dev/serial0"
 *     BaudRate = 115200
 *     Signals = {
 *          Signals = {
 *              ADCTime = {
 *                  Type = uint32
 *              }
 *              PPS1Time = {
 *                  Type = uint32
 *              }
 *              PPS2Time = {
 *                  Type = uint32
 *              }
 *              ADC1Data = {
 *                  Type = uint16
 *              }
 *              ADC2Data = {
 *                  Type = uint16
 *              }
 *          }
 *    }
 * }
 * </pre>
 * 
 * The Port is the path to the serial port device within the filesystem. The BaudRate is the data
 * speed that the DataSource will configure. 
 * 
 * The following signals must be defined:
 * 
 *  - ADCTime: The STM32 timestamp of the ADC samples
 *  - PPS1Time: The STM32 timestamp of the first PPS signal
 *  - PPS2Time: The STM32 timestamp of the second PPS signal
 *  - ADC1Data: The first ADC data value
 *  - ADC2Data: The second ADC data value
 * 
 * Note that any characteristics of the serial port which are not covered by the configuration 
 * above (e.g. parity, number of data bits) will retain whatever default settings are defined
 * for the serial port.
 */
class STM32 : public MARTe::DataSourceI {
 public:

    CLASS_REGISTER_DECLARATION();
    
    STM32();
    virtual ~STM32();

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
     * File descriptor for the serial port
     */
    int serial_fd;

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
     * Output signals
     */
    ADCSignals out_signals;
};

} // namespace MFI

#endif // MFI_DATASOURCES_STM32_H_
