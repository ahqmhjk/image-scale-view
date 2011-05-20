# !/bin/bash

set -e

if ! -d obj 
then
	mkdir obj
fi

make

rm obj -rf


exit 0
