#!/bin/bash

rm file*.img
rm -r rootdir
rm -r mountdir

for((i=1;i<$1+1;i=i+1))
do
	losetup -d /dev/loop$i
done
