#! /bin/sh

#================================================================
# File: set_vars.cs
#
# Usage:  . set_vars.sh
#
# This script will attempt to download and install/build all of the
# open-source repositories required to run the MatchLib toolkit examples
# without a Catapult installation.

# It will create local cached builds:
#    ./sysclocal
#    ./matchlib_connections
#    ./matchlib
#    ./preprocessor
#    ./rapidjson
#    ./ac_types
#    ./ac_math
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

# Configure Boost Preprocessor
if [ ! -d /Xusr/include/boost/preprocessor/arithmetic ]; then
  if [ ! -d ./boost_home/include/boost ]; then
    git clone http://github.com/boostorg/preprocessor
    git clone http://github.com/boostorg/static_assert
    mkdir -p boost_home/include/boost
    mv preprocessor/include/boost/* boost_home/include/boost
    mv static_assert/include/boost/* boost_home/include/boost
    rm -rf preprocessor static_assert
  fi
  BOOST_HOME=`pwd`/boost_home
else
  BOOST_HOME=/usr
fi
export BOOST_HOME

# Configure RapidJSON
if [ ! -d ./rapidjson ]; then
 git clone http://github.com/Tencent/rapidjson
endif
RAPIDJSON_HOME=`pwd`/rapidjson
export RAPIDJSON_HOME

# Configure AC Datatypes
if [ ! -d ./ac_types ]; then
  echo "Downloading AC_Types..."
  git clone http://github.com/hlslibs/ac_types.git
fi
AC_TYPES=`pwd`/ac_types
export AC_TYPES

# Configure AC Math
if [ ! -d ./ac_math ]; then
  echo "Downloading AC_Math..."
  git clone http://github.com/hlslibs/ac_math.git
fi
AC_TYPES=`pwd`/ac_math
export AC_MATH

# Configure AC Simutils
if [ ! -d ./ac_simutils ]; then
  echo "Downloading AC_Simutils..."
  git clone http://github.com/hlslibs/ac_simutils.git
fi
AC_SIMUTILS=`pwd`/ac_simutils
export AC_SIMUTILS

