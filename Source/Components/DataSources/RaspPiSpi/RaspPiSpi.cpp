#include "RaspPiSpi.h"
#include "RaspPiSpiReader.h"
#include "RaspPiSpiWriter.h"
#include "AdvancedErrorManagement.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <unistd.h>

using namespace MARTe;

namespace MFI {

RaspPiSpi::RaspPiSpi() : DataSourceI() {
    device = "";
    spi_fd = 0;
    msg_len = 0;
    speed = 0;
    phase = 0;
    polarity = 0;
    cs_active_level = 0;
    tx_buffer = NULL_PTR(uint8 *);
    rx_buffer = NULL_PTR(uint8 *);
}

RaspPiSpi::~RaspPiSpi() {
    if (spi_fd > 0) {
        close(spi_fd);
    }

    if (!tx_buffer) {
        delete [] tx_buffer;
    }

    if (!rx_buffer) {
        delete [] rx_buffer;
    }
}

bool RaspPiSpi::Initialise(StructuredDataI &data) {
    bool ok = DataSourceI::Initialise(data);

    if (ok) {
        ok = data.Read("Device", device);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The SPI device should be specified.");
        }
    }

    if (ok) {
        ok = data.Read("Message_Length", msg_len);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The SPI message length should be specified.");
        }
    }

    if (ok) {
        ok = data.Read("Speed", speed);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The SPI speed should be specified.");
        }
    }
  
    if (ok) {
        ok = data.Read("Clock_Phase", phase);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The SPI clock phase should be specified.");
        }
    }

    if (ok) {
        ok = data.Read("Clock_Polarity", polarity);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The SPI clock polarity should be specified.");
        }
    }

    if (ok) {
        ok = data.Read("CS_Active_Level", cs_active_level);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The SPI chip select active level should be specified.");
        }
    }

    if (ok) {
        // Open the SPI device
        spi_fd = open(device.Buffer(), O_RDWR);
        ok = !(spi_fd < 0);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::InitialisationError, "Failed to open SPI device");
        }

        // Set the SPI Mode
        if (ok) {
            uint32 mode = 0;
            if (phase) {
                mode |= SPI_CPHA;
            }
            if (polarity) {
                mode |= SPI_CPOL;
            }
            if (cs_active_level) {
                mode |= SPI_CS_HIGH;
            }
            ok = (ioctl(spi_fd, SPI_IOC_WR_MODE32, &mode) != -1);
	        if (!ok) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Failed to set SPI mode");
            }
        }

        // Set the bits per word
        if (ok) {
            uint32 bits = 0; // 8 bits per word 
            ok = (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) != -1);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Failed to set SPI bits per word");
            }
        }

        // Set the speed
        if (ok) {
            ok = (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) != -1);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::InitialisationError, "Failed to set SPI speed");
            }
        }

        if (ok) {
            tx_buffer = new uint8[msg_len];
            rx_buffer = new uint8[msg_len];
            if (!tx_buffer || !rx_buffer) {
                ok = false;
            }
        }
    }

    return ok;
}

bool RaspPiSpi::Synchronise() {
    struct spi_ioc_transfer tr;
    
    // Note: not at all clear on what some of these fields do
    tr.tx_buf = (__u64) &tx_buffer[0];
    tr.rx_buf = (__u64) &rx_buffer[0];
    // Add support for getting messages from the 
    tr.len = msg_len;
    tr.delay_usecs = 0;
    tr.speed_hz = speed,
    tr.bits_per_word = 8;
	tr.cs_change = 1;
	tr.tx_nbits = 8;
	tr.rx_nbits = 8;
	tr.pad = 0;

    int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
	
    if (ret < 1)
		return false;
    
    return true;
}

bool RaspPiSpi::AllocateMemory() {
    // To do: check if any memory should be allocated here
    return true;
}

uint32 RaspPiSpi::GetNumberOfMemoryBuffers() {
    // To do: chek if this is correct for this data source
    return 1u;
}

bool RaspPiSpi::GetSignalMemoryBuffer(const MARTe::uint32 signalIdx,
                                      const MARTe::uint32 bufferIdx,
                                      void *&signalAddress) {
    // To do: check if this is the correct implementation
    signalAddress = NULL;

    return true;
}

const char8 * RaspPiSpi::GetBrokerName(StructuredDataI &data, const SignalDirection direction) {
    if (direction == InputSignals) {
        return "RaspPiSpiReader";
    }
    else {
        return "RaspPiSpiWriter";
    }

    return "";
}

bool RaspPiSpi::SetConfiguredDatabase(StructuredDataI &data) {
    // To do: check what implementation is required here
    bool ret = DataSourceI::SetConfiguredDatabase(data);
    
    return ret;
}

bool RaspPiSpi::GetInputBrokers(ReferenceContainer &inputBrokers,
                                        const char8* const functionName,
                                        void * const gamMemPtr) {
    ReferenceT<RaspPiSpiReader> broker("RaspPiSpiReader");
    bool ret = broker.IsValid();
    if (ret) {
        ret = broker->Init(InputSignals, *this, functionName, gamMemPtr);
    }
    if (ret) {
        ret = inputBrokers.Insert(broker);
    }
    
    return ret;
    // To do: check the implementation of DataSourceI::GetInputBrokers - do we need it?
}

bool RaspPiSpi::GetOutputBrokers(ReferenceContainer &outputBrokers,
                                         const char8* const functionName,
                                         void * const gamMemPtr) {
    ReferenceT<RaspPiSpiWriter> broker("RaspPiSpiWriter");
    bool ret = broker.IsValid();
    if (ret) {
        ret = broker->Init(OutputSignals, *this, functionName, gamMemPtr);
    }
    if (ret) {
        ret = outputBrokers.Insert(broker);
    }

    return ret;
    // To do: check the implementation of DataSourceI::GetOutputBrokers - do we need it?
}

bool RaspPiSpi::PrepareNextState(const char8 * const currentStateName,
                                         const char8 * const nextStateName) {
    return true;
}

bool RaspPiSpi::WriteTxBuffer(uint8 const * const buffer, uint32 length) {
    if (length > msg_len) {
        return false;
    }

    for (uint32 i = 0; i < length; ++i) {
        tx_buffer[i] = buffer[i];
    }

    return true;
}

bool RaspPiSpi::ReadRxBuffer(uint8 * buffer, uint32 &length) {
    for (uint32 i = 0; i < msg_len; ++i) {
        buffer[i] = rx_buffer[i];
    }
    length = msg_len;
    
    return true;
}

CLASS_REGISTER(RaspPiSpi, "1.0");

}  // namespace MFI
