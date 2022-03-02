#!/bin/bash
#Arguments -l LOADER -f FILENAME -m MESSAGE | -s STATE [-d cgdb|strace]
#-l LOADER=The Loader to use
#-f FILENAME=MARTe configuration file
#-m MESSAGE=Start message
#-s STATE=RealTimeApplication first state 
#-d cgdb=Run with cgdb
#-d strace=Run with strace
#
# This application launcher script has been adapted from the example script at
# https://vcis.f4e.europa.eu/marte2-docs/master/html/core/app/app.html

# Quick and dirty check of the architecture
ARCH=`uname -m`
if [[ "$ARCH" == "x86_64" ]]; then
    TARGET=x86-linux
else
    TARGET=armv8-linux
fi

# Run with cgdb or strace?
DEBUG=""

# Consume input arguments
while [[ $# -gt 1 ]]
do
key="$1"

case $key in
    -l|--loader)
    LOADER="$2"
    shift # past argument
    ;;
    -f|--file)
    FILE="$2"
    shift # past argument
    ;;
    -m|--message)
    MESSAGE="$2"
    shift # past argument
    ;;
    -s|--state)
    STATE="$2"
    shift # past argument
    ;;
    -d|--debug)
    DEBUG="$2"
    shift # past argument
    ;;    
    --default)
    DEFAULT=YES
    ;;
    *)
            # unknown option
    ;;
esac
shift # past argument or value
done

if [ -z ${MARTe2_DIR+x} ]; then 
    echo "MARTe2_DIR environment variable is not set"
    echo "Terminating"
    exit 1
fi
if [ -z ${MARTe2_Components_DIR+x} ]; then 
    echo "MARTe2_Components_DIR environment variable is not set"
    echo "Terminating"
    exit 1
fi

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_DIR/Build/$TARGET/Core/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/LinuxTimer/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/LoggerDataSource/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/DAN/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/FileDataSource/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/LinkDataSource/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/UDP/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/RealTimeThreadSynchronisation/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/RealTimeThreadAsyncBridge/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/ConstantGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/ConversionGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/IOGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/FilterGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/HistogramGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/SSMGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/TriggerOnChangeGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/WaveformGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/Interfaces/SysLogger/

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/Components/DataSources/RaspPiSerial
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/Components/DataSources/GPS
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/Components/DataSources/STM32
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/Components/GAMs/UTCTimestampingGAM
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/Components/Lib/DataSourceSignalChecker
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/Components/Lib/GAMSignalChecker

echo $LD_LIBRARY_PATH | sed -e's/:/\n\t/g'
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH

MARTeAPP=$MARTe2_DIR/Build/$TARGET/App/MARTeApp.ex

# Start with cgdb or with strace
if [ "$DEBUG" = "cgdb" ]
then
    if [ -z ${STATE+x} ]; then
        cgdb --args $MARTeAPP -l $LOADER -f $FILE -m $MESSAGE
    else
        cgdb --args $MARTeAPP -l $LOADER -f $FILE -s $STATE
    fi
elif [ "$DEBUG" = "gdb" ]
then 
    if [ -z ${STATE+x} ]; then
        gdb -q -iex "set auto-load safe-path $(pwd)" --args $MARTeAPP -l $LOADER -f $FILE -m $MESSAGE | tee gdb.log
    else
        gdb -q -iex "set auto-load safe-path $(pwd)" --args $MARTeAPP -l $LOADER -f $FILE -s $STATE | tee gdb.log
    fi
elif [ "$DEBUG" = "strace" ]
then
    if [ -z ${STATE+x} ]; then
        strace -o/tmp/strace.err $MARTeAPP -l $LOADER -f $FILE -m $MESSAGE
    else
        strace -o/tmp/strace.err $MARTeAPP -l $LOADER -f $FILE -s $STATE
    fi
else
    if [ -z ${STATE+x} ]; then
        $MARTeAPP -l $LOADER -f $FILE -m $MESSAGE
    else
        $MARTeAPP -l $LOADER -f $FILE -s $STATE 
    fi
fi
