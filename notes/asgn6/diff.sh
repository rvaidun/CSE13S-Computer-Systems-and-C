#!/bin/bash

FILES=../../../resources/corpora
MYREPO=../../asgn6
EXAMPLE=../../../resources/asgn6
(cd $MYREPO && make clean && make)
for f in $(find $FILES -type f)
do
	echo "Checking difference for $(basename $f)"
	diff <($MYREPO/encode < $f | hd) <($EXAMPLE/encode < $f | hd)
	diff <($MYREPO/encode < $f | $MYREPO/decode) $f
done
(cd $MYREPO && make clean)
(cd $EXAMPLE && git clean -f)