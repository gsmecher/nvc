set -xe

# Plugins loaded with --load apply to the simulation that follows a Tcl
# script as well as the simulation in the script
nvc --load=vhpi_test -a $TESTDIR/regress/cmdline24.vhd -e cmdline24 \
    --do $TESTDIR/regress/cmdline24.tcl -r --stop-time=10ns
