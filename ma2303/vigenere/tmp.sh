#!/bin/bash
output=0
for x in {16,25,13,7,14,1,23,25,3}; do
	let "output = (21*($x-7))%26"
	echo -n $output" "
done
echo ""
