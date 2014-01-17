#!/bin/bash

for((i=1;i<$2+1;i=i+1))
do
	cp $1 ./mountdir/$1.i
done

