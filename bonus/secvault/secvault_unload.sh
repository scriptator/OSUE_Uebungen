#!/bin/sh
module="secvault"
device="secvault"

# invoke rmmod with all arguments we got
/sbin/rmmod $module $* || exit 1

# remove stale nodes
rm -f /dev/${device}[1-4] /dev/sv_ctl