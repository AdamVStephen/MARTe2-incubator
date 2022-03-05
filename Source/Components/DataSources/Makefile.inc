OBJSX= 

SPB = ComtradeDataSource.x GPS.x STM32.x

ifeq ($(TARGET), rpi4)
SPB += RaspPiSpi.x RaspPiSerial.x 
endif

ROOT_DIR=../../..
PACKAGE=Components

MAKEDEFAULTDIR=$(MARTe2_DIR)/MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

LIBRARIES_STATIC=$(BUILD_DIR)/GPS/GPS$(LIBEXT)
LIBRARIES_STATIC+=$(ROOT_DIR)/Build/$(TARGET)/Components/Lib/DataSourceSignalChecker/DataSourceSignalChecker$(LIBEXT)

ifeq ($(TARGET), rpi4)
LIBRARIES_STATIC+=$(BUILD_DIR)/RaspPiSerial/RaspPiSerial$(LIBEXT)
LIBRARIES_STATIC+=$(BUILD_DIR)/RaspPiSpi/RaspPiSpi$(LIBEXT)
endif

all: $(OBJS)  $(SUBPROJ)  \
    $(BUILD_DIR)/DataSources$(LIBEXT) \
    $(BUILD_DIR)/DataSources$(DLLEXT)
	echo  $(OBJS)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)
