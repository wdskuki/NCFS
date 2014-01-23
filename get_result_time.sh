#!/bin/bash

[ "$#" -lt 4 ] && echo "Usage: bash get_result_time.sh (new device) (fail device id) (result file path) (circle num) " && exit 0


for((i=1;i<$4+1;i=i+1))
do
	./recover $1 $2 | grep "Elapsed" | awk '{print $4}' >> $3
done

