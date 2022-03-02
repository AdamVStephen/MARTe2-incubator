OBJSX=

SPB=Source/Lib.x \
    Source/Components/DataSources.x \
    Source/Components/GAMs.x

SPB += Test/Components/DataSources.x

SPB += Test/GTest.x

MAKEDEFAULTDIR=$(MARTe2_DIR)/MakeDefaults

ROOT_DIR=.

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

all: $(OBJS) $(SUBPROJ) check-env
	echo  $(OBJS)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

check-env:
ifndef MARTe2_DIR
    $(error MARTe2_DIR is undefined)
endif
ifndef MARTe2_Components_DIR
    $(error MARTe2_Components_DIR is undefined)
endif