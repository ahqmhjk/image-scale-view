# !/bin/bash

set -e

if [ ! -d obj ]
then
	mkdir obj
fi

GTK3_PATH=/usr/include/gtk-3.0/

if test -d $GTK3_PATH
then
	make "GTK=3"
else
	make
fi

rm obj -rf


exit 0
