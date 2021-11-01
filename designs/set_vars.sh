#! /bin/sh

#================================================================
# File: set_vars.sh
#
# Usage: . set_vars.sh
#
# This script will attempt to download and install/build all of the
# open-source repositories required to run the MatchLib toolkit examples
# without a Catapult installation.

# It will create local cached builds:
#    ./sysclocal
#    ./matchlib_connections
#    ./matchlib
#    ./ac_types
#    ./ac_simutils

# It also intentionally unsets CATAPULT_HOME and MGC_HOME
CATAPULT_HOME=
export CATAPULT_HOME
MGC_HOME=
export MGC_HOME

#================================================================

# Configure SystemC 2.3.3 (download and build if needed)
SYSCVER=2.3.3
if [ ! -d ./systemc-$SYSCVER ]; then
  echo "Downloading and building SystemC $SYSCVER ..."
  rm -f systemc-$SYSCVER.tar.gz
  # Download and build SystemC
  wget --no-check-certificate https://www.accellera.org/images/downloads/standards/systemc/systemc-$SYSCVER.tar.gz
  /bin/gtar zxf ./systemc-$SYSCVER.tar.gz
  CXX="g++ -std=c++11"
  export CXX
  cd systemc-$SYSCVER
  rm -rf objdir
  mkdir objdir
  cd objdir
  ../configure
  make
  make install
  cd ../..
  rm -rf systemc-$SYSCVER.tar.gz
fi
SYSTEMC_HOME=`pwd`/systemc-$SYSCVER
export SYSTEMC_HOME

# Configure MatchLib Connections
if [ ! -d ./matchlib_connections ]; then
  echo "Downloading MatchLib Connections..."
  git clone http://github.com/hlslibs/matchlib_connections.git
fi
CONNECTIONS_HOME=`pwd`/matchlib_connections
export CONNECTIONS_HOME

# Configure MatchLib
if [ ! -d ./matchlib ]; then
 git clone http://github.com/NVlabs/matchlib.git
fi
MATCHLIB_HOME=`pwd`/matchlib
export MATCHLIB_HOME

# Configure AC Datatypes
if [ ! -d ./ac_types ]; then
  echo "Downloading AC_Types..."
  git clone http://github.com/hlslibs/ac_types.git
fi
AC_TYPES=`pwd`/ac_types
export AC_TYPES

# Configure AC Simutils
if [ ! -d ./ac_simutils ]; then
  echo "Downloading AC_Simutils..."
  git clone http://github.com/hlslibs/ac_simutils.git
fi
AC_SIMUTILS=`pwd`/ac_simutils
export AC_SIMUTILS

