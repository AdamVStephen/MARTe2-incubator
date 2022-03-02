#include "RaspPiSpiWriter.h"
#include "RaspPiSpi.h"
#include "AdvancedErrorManagement.h"

using namespace MARTe;

namespace MFI {

RaspPiSpiWriter::RaspPiSpiWriter() : MemoryMapOutputBroker() {
    spi = NULL;
}

RaspPiSpiWriter::~RaspPiSpiWriter() {

}

bool RaspPiSpiWriter::Init(const SignalDirection direction,
                           DataSourceI &dataSourceIn,
                           const char8 * const functionName,
                           void * const gamMemoryAddress) {
    bool ret = MemoryMapOutputBroker::Init(direction, dataSourceIn, functionName, gamMemoryAddress);
    if (ret) {
        spi = dynamic_cast<RaspPiSpi*>(dataSource);
        ret = (spi != NULL);
        if (!ret) {
            REPORT_ERROR(ErrorManagement::FatalError, "Failed dynamic_cast from DataSourceI* to RaspPiSpi*");
        }
    }
    
    return ret;
}

bool RaspPiSpiWriter::Execute() {
    bool ret = true;
    
    for (uint32 n = 0u; (n < numberOfCopies) && (ret); n++) {
        if (copyTable != NULL_PTR(MemoryMapBrokerCopyTableEntry *)) {
            // To do: replace this with implementation using signal memory buffers
            spi->WriteTxBuffer((uint8 *)copyTable[n].gamPointer, copyTable[n].copySize);
        }
    }
    
    return ret;
}

CLASS_REGISTER(RaspPiSpiWriter, "1.0");

}  // namespace MFI
