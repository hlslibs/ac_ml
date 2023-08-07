#! /bin/csh -f

#================================================================
# File: set_vars.csh
#
# Usage:  source set_vars.csh
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
unsetenv CATAPULT_HOME
unsetenv MGC_HOME

#================================================================

# Configure SystemC 2.3.3 (download and build if needed)
set SYSCVER=2.3.3
if (! -e ./systemc-$SYSCVER ) then
  echo "Downloading and building SystemC $SYSCVER ... --no-check-certificate"
  rm -f systemc-$SYSCVER.tar.gz
  # Download and build SystemC
  wget --no-check-certificate https://www.accellera.org/images/downloads/standards/systemc/systemc-$SYSCVER.tar.gz
  /bin/gtar zxf ./systemc-$SYSCVER.tar.gz
  setenv CXX "g++ -std=c++11"
  cd systemc-$SYSCVER
  rm -rf objdir
  mkdir objdir
  cd objdir
  ../configure
  make
  make install
  cd ../..
  rm -rf systemc-$SYSCVER.tar.gz
endif
setenv SYSTEMC_HOME `pwd`/systemc-$SYSCVER

# Configure MatchLib Connections
if (! -e ./matchlib_connections) then
  echo "Downloading MatchLib Connections..."
  git clone http://github.com/hlslibs/matchlib_connections.git
endif
setenv CONNECTIONS_HOME `pwd`/matchlib_connections

# Configure MatchLib
if (! -e ./matchlib ) then
 git clone http://github.com/NVlabs/matchlib.git
endif
setenv MATCHLIB_HOME `pwd`/matchlib

# Configure Boost Preprocessor and Static Assert
# (We can't depend on the linux install to have boost, so always download minimum)
if (! -e /Xusr/include/boost/preprocessor/arithmetic ) then
  if (! -e ./boost_home/include/boost) then
    git clone http://github.com/boostorg/preprocessor
    git clone http://github.com/boostorg/static_assert
    mkdir -p boost_home/include/boost
    mv preprocessor/include/boost/* boost_home/include/boost
    mv static_assert/include/boost/* boost_home/include/boost
    rm -rf preprocessor static_assert
  endif
  setenv BOOST_HOME `pwd`/boost_home
else
  setenv BOOST_HOME /usr
endif

# Configure RapidJSON
if (! -e ./rapidjson ) then
 git clone http://github.com/Tencent/rapidjson
endif
setenv RAPIDJSON_HOME `pwd`/rapidjson

# Configure AC Datatypes
if (! -e ./ac_types) then
  echo "Downloading AC_Types..."
  git clone http://github.com/hlslibs/ac_types.git
endif
setenv AC_TYPES `pwd`/ac_types

# Configure AC Math
if (! -e ./ac_math) then
  echo "Downloading AC_Math..."
  git clone http://github.com/hlslibs/ac_math.git
endif
setenv AC_MATH `pwd`/ac_math

# Configure AC Simutils
if (! -e ./ac_simutils) then
  echo "Downloading AC_Simutils..."
  git clone http://github.com/hlslibs/ac_simutils.git
endif
setenv AC_SIMUTILS `pwd`/ac_simutils

