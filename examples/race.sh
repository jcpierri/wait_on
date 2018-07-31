#!/bin/sh
# $Id: race.sh,v 1.2 2003/05/25 13:07:35 andrew Exp $

FILE1=1
FILE2=2

wait_on -t 60 -i $FILE1 $FILE2

case $? in
0)
	echo Neither the file $FILE1 or the file $FILE2 changed.
	;;
1)
	echo File $FILE1 has been changed.
	;;
2)
	echo File $FILE2 has been changed.
	;;
esac
