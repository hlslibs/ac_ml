#! /bin/csh -f

#================================================================
# File: set_rocket_vars.csh
#
# Usage:  source set_rocket_vars.csh
#
# This script will download the open-source Rocket Chip package

# It will create local cached install:
#    ./rocket-chip


#================================================================

# Configure Rocket Chip
if (! -e ./rocket-chip) then
  echo "Downloading Rocket Chip..."
  git clone http://github.com/chipsalliance/rocket-chip.git
endif
setenv ROCKET_CORE_HOME `pwd`/rocket-chip

