SPB    = GPS.x

PACKAGE=Components
ROOT_DIR=../../..
MAKEDEFAULTDIR=$(MARTe2_DIR)/MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

#LIBRARIES_STATIC=$(BUILD_DIR)/GPS/GPS$(LIBEXT)
LIBRARIES_STATIC=$(BUILD_DIR)/GPS/GPSTest$(LIBEXT)

all: $(OBJS) $(SUBPROJ) \
    $(BUILD_DIR)/DataSourcesTests$(LIBEXT)
	echo  $(OBJS)

#include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)
