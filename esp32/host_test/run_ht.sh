#!/bin/sh

tmpfile=`mktemp`
./build/host_test.elf > $tmpfile &
sleep 1
kill $!
status=`tail -n 1 $tmpfile`
rm $tmpfile
echo $status | grep -Eq 'PASSED'
exit $?
