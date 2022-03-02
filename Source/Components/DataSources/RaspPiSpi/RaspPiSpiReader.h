#ifndef MFI_DATASOURCES_RASPPISPIREADER_H_
#define MFI_DATASOURCES_RASPPISPIREADER_H_

#include "CompilerTypes.h"
#include "MemoryMapInputBroker.h"

namespace MFI {

class RaspPiSpi;

class RaspPiSpiReader : public MARTe::MemoryMapInputBroker {
 public:
    CLASS_REGISTER_DECLARATION();

    RaspPiSpiReader();
    virtual ~RaspPiSpiReader();
    
    virtual bool Init(const MARTe::SignalDirection direction,
                      MARTe::DataSourceI &dataSourceIn,
                      const MARTe::char8 * const functionName,
                      void * const gamMemoryAddress);

    virtual bool Execute();

 private:
    RaspPiSpi* spi;
};

}  // namespace MFI

#endif  // MFI_DATASOURCES_RASPPISPIREADER_H_