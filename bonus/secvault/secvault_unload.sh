#!/bin/sh
module="secvault"
device="sv_data"

# invoke rmmod with all arguments we got
/sbin/rmmod $module $* || exit 1

# remove stale nodes
rm -f /dev/${device}[0-3] /dev/sv_ctl