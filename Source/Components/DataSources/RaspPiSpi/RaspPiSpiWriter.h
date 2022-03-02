#ifndef MFI_DATASOURCES_RASPPISPIWRITER_H_
#define MFI_DATASOURCES_RASPPISPIWRITER_H_

#include "CompilerTypes.h"
#include "MemoryMapOutputBroker.h"

namespace MFI {

class RaspPiSpi;

class RaspPiSpiWriter : public MARTe::MemoryMapOutputBroker {
 public:
    CLASS_REGISTER_DECLARATION();

    RaspPiSpiWriter();
    virtual ~RaspPiSpiWriter();
    
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
