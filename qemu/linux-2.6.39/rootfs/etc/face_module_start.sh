#!/bin/sh
PROG=$(ps | grep FM20 | grep -v "grep")
s=0;  
max=5  
while true; do
	PROG=$(ps | grep FM20 | grep -v "grep")

	#echo $PROG
	
	if [ -z "$PROG" ]; then
		echo "oh! not find the program. start up it..."
		umount /lib -l
		mount -t ubifs ubi0_0 /lib/ -r
		/lib/home/hanvon/app.sh &
		let s++;  
		if [ $s -gt $max ]; then
			echo "reboot"
			reboot
		fi
	else
		#echo "ok, find the program: " $PROG
		sleep 3
		continue
	fi

	sleep 5 
done
