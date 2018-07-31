#!/bin/sh
# $Id: watch_folder.sh,v 1.1.1.1 2002/01/29 16:35:36 andrew Exp $

SOURCE=/ftp/root/incoming
DESTINATION=/ftp/uploaded_files

while :; do
	mv $SOURCE/* $DESTINATION
	wait_on -w $SOURCE
done
