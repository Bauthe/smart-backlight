#!/bin/sh

pipe=/tmp/smart-backlight
file=/sys/class/backlight/acpi_video0/brightness

if [[ ! -p $pipe ]]
then mkfifo $pipe
fi

chmod 777 $pipe

read line < $pipe

if [ "$line" = setpermissions ]
then
	chgrp smart-backlight $file
	chmod g+w $file
	echo complete > $pipe

	rm $pipe
fi

exit 0
