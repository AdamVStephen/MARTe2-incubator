#include "RaspPiSpiReader.h"
#include "RaspPiSpi.h"
#include "AdvancedErrorManagement.h"

using namespace MARTe;

namespace MFI {

RaspPiSpiReader::RaspPiSpiReader() : MemoryMapInputBroker() {
    spi = NULL;
}

RaspPiSpiReader::~RaspPiSpiReader() {

}


bool RaspPiSpiReader::Init(const SignalDirection direction,
                           DataSourceI &dataSourceIn,
                           const char8 * const functionName,
                           void * const gamMemoryAddress) {
    bool ret = MemoryMapInputBroker::Init(direction, dataSourceIn, functionName, gamMemoryAddress);
    if (ret) {
        spi = dynamic_cast<RaspPiSpi*>(dataSource);
        ret = (spi != NULL);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::FatalError, "Failed dynamic_cast from DataSourceI* to RaspPiSpi*");
        }
    }
    
    return ret;
}

bool RaspPiSpiReader::Execute() {
    bool ret = true;
    
    // To do: add a call to RaspPiSpi::Synchronise to trigger SPI transaction
    
    for (uint32 n = 0u; (n < numberOfCopies) && (ret); n++) {
        if (copyTable != NULL_PTR(MemoryMapBrokerCopyTableEntry *)) {
            // To do: replace this with implementation using signal memory buffers
            spi->ReadRxBuffer((uint8 *)copyTable[n].gamPointer, copyTable[n].copySize);
        }
    }
    
    return ret;
}

CLASS_REGISTER(RaspPiSpiReader, "1.0");

}  // namespace MFI
