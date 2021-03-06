#!/bin/bash

source /data/src/docker/joynr-base/scripts/global.sh

DBUS='OFF'
GCOV='OFF'
JOBS=8

function usage
{
    echo "usage: cpp-clean-build.sh [--dbus ON|OFF --gcov ON|OFF --jobs X]"
    echo "default dbus is $DBUS, gcov is $GCOV, jobs is $JOBS"
}

while [ "$1" != "" ]; do
    case $1 in
        --dbus )                shift
                                DBUS=$1
                                ;;
        --gcov )                shift
                                GCOV=$1
                                ;;
        --jobs )                shift
                                JOBS=$1
                                ;;
        * )                     usage
                                exit 1
    esac
    shift
done

log "CPP CLEAN BUILD DBUS: $DBUS GCOV: $GCOV JOBS: $JOBS"

log "Enable core dumps"
ulimit -c unlimited

START=$(date +%s)


log "ENVIRONMENT"
env

log "CLEAN BUILD DIRECTORY"
rm -rf ~/.cmake/packages
rm -rf /data/build/joynr
mkdir /data/build/joynr

cd /data/build/joynr

log "RUN CMAKE"

# fail on first error
set -e
cmake -DUSE_DBUS_COMMONAPI_COMMUNICATION=$DBUS \
      -DENABLE_GCOV=$GCOV \
      -DPYTHON_EXECUTABLE=/usr/bin/python \
      -DJOYNR_SERVER=localhost:8080 \
      -DCMAKE_BUILD_TYPE=Debug /data/src/cpp \
      -DUSE_PLATFORM_LIBWEBSOCKETS=ON \
      -DUSE_PLATFORM_SPDLOG=ON \
      -DUSE_PLATFORM_JSMN=ON \
      -DUSE_PLATFORM_GTEST_GMOCK=ON


if [ "$GCOV" == "ON" ] ; then
    echo "run coverage build"
    make -j $JOBS UnitCoverageTarget
fi

log "BUILD C++ JOYNR"
make -j $JOBS
log "BUILD C++ JOYNR DOXYGEN DOCUMENTATION"
log "doxygen is disabled"
#make doxygen

tar czf joynr-clean-build.tar.gz bin

END=$(date +%s)
DIFF=$(( $END - $START ))
log "C++ build time: $DIFF seconds"
