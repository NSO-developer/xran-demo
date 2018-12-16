#!/bin/bash

for yang in `ls -1 *.yang |grep -v 'ann.yang'`
do
	echo $i
	yang_ann=`echo $yang |cut -d"." -f1`"-ann.yang"
	#echo $yang_ann
	
	if [ -f "$yang_ann" ]
	then
		confdc -c -a $yang_ann $yang
	else
		confdc -c $yang
	fi
done


