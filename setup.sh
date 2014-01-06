#!/bin/bash
CMDLN_ARGS="$@" # Command line arguments for this script
export CMDLN_ARGS

# Run this script as root if not already.
chk_root () {

  if [ ! $( id -u ) -eq 0 ]; then
	echo "Please run as root"
	exit 0
#    echo "Please enter root's password."
#    exec su -c "${0} ${CMDLN_ARGS}" # Call this prog as root
#    exit ${?}  # sice we're 'execing' above, we wont reach this exit
               # unless something goes wrong.
  fi

}

chk_root

[ "$#" -lt 2 ] && echo "Usage: bash setup (number of disks) (size of one disk MB)" && exit 0
fusermount -u mountdir
rm file0*.img
rm -r rootdir
rm -r mountdir
[ "$1" -gt 7 ] && echo "Too many disks, support up to 7 disks" && exit 0
mkdir rootdir
mkdir mountdir
for((i=1;i<$1+1;i=i+1))
do
	losetup -d /dev/loop$i
	dd if=/dev/zero of=file0$i.img bs=1M count=$2
	losetup /dev/loop$i file0$i.img
done
rm raid_health
touch raid_health
for((i=0;i<$1;i=i+1))
do
echo "0"								>>raid_health
done
rm raid_metadata
touch raid_metadata
