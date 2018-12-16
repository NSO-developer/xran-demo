#!/bin/bash

context=$2
i=1

for a in "$@" ; do
	if [ $i -ge 3 ]; then
        	if [ $i -eq 3 ]; then
                	key1=$a
        	fi
		if [ $i -eq 4 ]; then
                	val1=$a
        	fi
                if [ $i -eq 5 ]; then
                        key2=$a
                fi
                if [ $i -eq 6 ]; then
                        val2=$a
                fi
        fi
	i=`expr $i + 1`
done

date=`date '+%Y-%m-%d %H:%M:%S %:zZ'`

echo "$date --- Supervision-Watchdog-Reset RPC 'Invoked from $2.'" >> /home/ubuntu/rpc/supervision-watchdog-reset.result

if [ -n "$val1" ]; then
	printf "\t\t\t\tWith Input: %s=%s\n" $key1 $val1 >> /home/ubuntu/rpc/supervision-watchdog-reset.result 
fi
if [ -n "$val2" ]; then
        printf "\t\t\t\tWith Input: %s=%s\n" $key2 $val2 >> /home/ubuntu/rpc/supervision-watchdog-reset.result
fi

echo $key1"="$val1 > /home/ubuntu/rpc/variables
echo $key2"="$val2 >> /home/ubuntu/rpc/variables

echo $val1 > /home/ubuntu/rpc/variables2
echo $val2 >> /home/ubuntu/rpc/variables2


#echo "supervision-watchdog-reset-result 'Invoked from $2 -- $date -- Successful'"
#echo "SUCCESS"
