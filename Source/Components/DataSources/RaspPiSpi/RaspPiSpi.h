#ifndef MFI_DATASOURCES_RASPPISPI_H_
#define MFI_DATASOURCES_RASPPISPI_H_

#include "DataSourceI.h"

namespace MFI {

const MARTe::uint32 TX_BUFFER_SIZE = 32;
const MARTe::uint32 RX_BUFFER_SIZE = 32;

// To do: add docs string explaining configuration
class RaspPiSpi : public MARTe::DataSourceI {

 public:

    CLASS_REGISTER_DECLARATION();

    RaspPiSpi();
    virtual ~RaspPiSpi();

    // Standard MARTe::DataSourceI functions

    virtual bool Initialise(MARTe::StructuredDataI &data);

    virtual bool Synchronise();

    virtual bool AllocateMemory();

    virtual MARTe::uint32 GetNumberOfMemoryBuffers();

    bool GetSignalMemoryBuffer(const MARTe::uint32 signalIdx,
                               const MARTe::uint32 bufferIdx,
                               void *&signalAddress);


    virtual const MARTe::char8 * GetBrokerName(MARTe::StructuredDataI &data, 
                                              const MARTe::SignalDirection direction);

    virtual bool SetConfiguredDatabase(MARTe::StructuredDataI &data);

    virtual bool GetInputBrokers(MARTe::ReferenceContainer &inputBrokers,
                                 const MARTe::char8* const functionName,
                                 void * const gamMemPtr);

    virtual bool GetOutputBrokers(MARTe::ReferenceContainer &outputBrokers,
                                  const MARTe::char8* const functionName,
                                  void * const gamMemPtr);

    virtual bool PrepareNextState(const MARTe::char8 * const currentStateName,
                                  const MARTe::char8 * const nextStateName);

    // Custom RaspPiSpi functions

    bool WriteTxBuffer(MARTe::uint8 const * const buffer, MARTe::uint32 length);

    bool ReadRxBuffer(MARTe::uint8 * buffer, MARTe::uint32 &length);

 private:
    /**
     * SPI device file path
     */
    MARTe::StreamString device;

    /**
     * SPI device file handle
     */
    MARTe::int32 spi_fd;

    /**
     * SPI message length
     */
    MARTe::uint32 msg_len;
    
    /**
     * SPI speed [Mbits/s]
     */
    MARTe::uint32 speed;

    /**
     * SPI clock phase
     */
    MARTe::uint32 phase;

    /**
     * SPI clock polarity
     */
    MARTe::uint32 polarity;

    /**
     * Chip select active level
     */
    MARTe::uint32 cs_active_level;

    /**
     * Transmit buffer
     */
    MARTe::uint8 *tx_buffer;

    /**
     * Receive buffer
     */
    MARTe::uint8 *rx_buffer;
};

} // namespace MFI

#endif // MFI_DATASOURCES_RASPPISPI_H_