#! /bin/sh

#================================================================
# File: set_rocket_vars.sh
#
# Usage: . set_rocket_vars.sh
#
# This script will download the open-source Rocket Chip package

# It will create local cached install:
#    ./rocket-chip


#================================================================

# Configure Rocket Chip
if [ ! -d ./rocket-chip ]; then
  echo "Downloading Rocket Chip..."
  git clone http://github.com/chipsalliance/rocket-chip.git
fi
export ROCKET_CORE_HOME=`pwd`/rocket-chip

