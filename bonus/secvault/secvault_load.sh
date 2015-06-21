#!/bin/sh
module="secvault"
device="secvault"
major="231"
mode="664"

# invoke insmod with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
/sbin/insmod ./$module.ko $* || exit 1

# remove stale nodes
rm -f /dev/${device}[1-4] /dev/sv_ctl

mknod /dev/sv_ctl c $major 0
mknod /dev/${device}1 c $major 1
mknod /dev/${device}2 c $major 2
mknod /dev/${device}3 c $major 3
mknod /dev/${device}4 c $major 4


# give appropriate group/permissions, and change the group.
# Not all distributions have staff, some have "wheel" instead.
group="staff"
chgrp $group /dev/${device}[1-4]
chmod $mode  /dev/${device}[1-4]
chgrp $group /dev/sv_ctl
chmod $mode  /dev/sv_ctl