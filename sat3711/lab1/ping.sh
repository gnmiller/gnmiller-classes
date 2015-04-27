#!/bin/bash

if [ $# -ne 1 -a $# -ne 2 ]; then
  echo "Usage:"
  echo "    trimmed_ping.sh <host> [num_pings]"
  echo "Results will be appeneded to ping.results in the current directory"
  exit 1
fi

numpings=60
echo "--------------------------------------------------------------------------------"
echo -n "Ping results for "
echo -n `date +%c`
echo " on "$1

#do the ping
pingresult=`ping -c $numpings $1`

#retrieve the right lines "$var" to preserve spacing
loss=`echo "$pingresult" | grep "packets"`
time=`echo "$pingresult" | grep "rtt"`

#get the avg time
avgtime=`echo $time | awk -F ' = ' '{ print $2 }' | awk -F '/' '{ print $2 }'`

#get the lossrate
lossrate=`echo $loss | awk -F ', ' '{ print $3 }' | awk -F ' ' '{ print $1 }'`

echo "Loss rate: "$lossrate
echo "Average time: "$avgtime
echo "--------------------------------------------------------------------------------"
